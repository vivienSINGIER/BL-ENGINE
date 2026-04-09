#include "PhysicSystem.h"
#include "../ECS/World.h"

void PhysicSystem::Update(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    const auto& collisions = m_narrowPhase->GetResults();
    const auto& triggers = m_narrowPhase->GetTriggerResults();

    if (collisions.empty() && triggers.empty())
        return;

    BuildFrameCollisionCache();

    for (int i = 0; i < kVelocityIterations; ++i)
        ResolveVelocities(_dt);

    for (int i = 0; i < kPositionIterations; ++i)
        ResolvePenetrations();

    NotifyScripts();
}

void PhysicSystem::BuildFrameCollisionCache()
{
    m_collisionCache.clear();

    if (m_narrowPhase == nullptr)
        return;

    const auto& collisions = m_narrowPhase->GetResults();
    m_collisionCache.reserve(collisions.size());

    for (const CollisionResult& collision : collisions)
    {
        const ContactInfo& originalContact = collision.contact;
        if (!originalContact.hit || originalContact.pointCount <= 0)
            continue;

        const ContactInfo reducedContact = BuildReducedContact(originalContact);

        CachedCollision cachedCollision;
        cachedCollision.collision = &collision;
        cachedCollision.contact = reducedContact;

        const XMFLOAT3 centerA = GetBodyCenter(collision.entityA);
        const XMFLOAT3 centerB = GetBodyCenter(collision.entityB);

        BuildContactPointCache(reducedContact, centerA, centerB, cachedCollision.points);
        m_collisionCache.push_back(cachedCollision);
    }
}

void PhysicSystem::ResolveVelocities(float _dt)
{
    if (_dt <= 0.0f)
        return;

    for (CachedCollision& cachedCollision : m_collisionCache)
        ResolveCollisionVelocities(cachedCollision);
}

void PhysicSystem::ResolveCollisionVelocities(CachedCollision& _cachedCollision)
{
    if (_cachedCollision.collision == nullptr)
        return;

    const CollisionResult& collision = *_cachedCollision.collision;
    const ContactInfo& contact = _cachedCollision.contact;

    if (!contact.hit || contact.pointCount <= 0)
        return;

    RigidBodyComponent* rigidA = GetRigidBody(collision.entityA);
    RigidBodyComponent* rigidB = GetRigidBody(collision.entityB);
    if (rigidA == nullptr || rigidB == nullptr)
        return;

    if (rigidA->massInverse + rigidB->massInverse <= 0.0f)
        return;

    MotionComponent& motionA = GetMotion(collision.entityA);
    MotionComponent& motionB = GetMotion(collision.entityB);

    if (motionA.isSleeping && motionB.isSleeping)
        return;

    WakeSleepingPair(motionA, motionB, *rigidA, *rigidB);

    SolveNormalImpulses(contact, motionA, motionB, *rigidA, *rigidB, _cachedCollision.points);
    SolveFrictionImpulses(contact, motionA, motionB, *rigidA, *rigidB, _cachedCollision.points);

    motionA.linearVelocity = Snap(motionA.linearVelocity, kLinearSnapThreshold);
    motionA.angularVelocity = Snap(motionA.angularVelocity, kAngularSnapThreshold);
    motionB.linearVelocity = Snap(motionB.linearVelocity, kLinearSnapThreshold);
    motionB.angularVelocity = Snap(motionB.angularVelocity, kAngularSnapThreshold);
}

void PhysicSystem::BuildContactPointCache(const ContactInfo& _contact, const XMFLOAT3& _centerA, const XMFLOAT3& _centerB, CachedContactPoint* _cache)
{
    for (int i = 0; i < _contact.pointCount; ++i)
    {
        CachedContactPoint& cached = _cache[i];
        cached = CachedContactPoint{};

        const ContactPoint& point = _contact.points[i];
        cached.rA = Subtract(point.position, _centerA);
        cached.rB = Subtract(point.position, _centerB);
        cached.active = true;
    }
}

void PhysicSystem::SolveNormalImpulses(
    const ContactInfo& _contact, MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB, CachedContactPoint* _cache)
{
    for (int i = 0; i < _contact.pointCount; ++i)
    {
        CachedContactPoint& cached = _cache[i];
        if (!cached.active)
            continue;

        const XMFLOAT3 vA = VelocityAtPoint(_motionA, cached.rA);
        const XMFLOAT3 vB = VelocityAtPoint(_motionB, cached.rB);
        const XMFLOAT3 relativeVelocity = Subtract(vA, vB);

        const float vn = Dot(relativeVelocity, _contact.normal);
        if (vn >= 0.0f)
        {
            cached.lastAppliedNormalImpulse = 0.0f;
            continue;
        }

        float restitution = Min(_rigidA.restitution, _rigidB.restitution);

        if (_contact.pointCount > 1 || fabsf(vn) < kRestitutionThreshold)
            restitution = 0.0f;

        const float effectiveMass =
            _rigidA.massInverse + _rigidB.massInverse +
            ComputeAngularMassTerm(cached.rA, _contact.normal, _rigidA.inertiaTensorWorldInverse) +
            ComputeAngularMassTerm(cached.rB, _contact.normal, _rigidB.inertiaTensorWorldInverse);

        if (effectiveMass <= 0.0f)
        {
            cached.lastAppliedNormalImpulse = 0.0f;
            continue;
        }

        float deltaImpulse = -(1.0f + restitution) * vn / effectiveMass;

        const float oldAccumulated = cached.accumulatedNormalImpulse;
        cached.accumulatedNormalImpulse = Max(0.0f, oldAccumulated + deltaImpulse);

        const float appliedImpulse = cached.accumulatedNormalImpulse - oldAccumulated;
        cached.lastAppliedNormalImpulse = appliedImpulse;

        if (appliedImpulse <= 0.0f)
            continue;

        const XMFLOAT3 impulse = Mul(_contact.normal, appliedImpulse);

        bool supportLikeContact =
            fabsf(_contact.normal.y) > 0.9f &&
            (_rigidA.type == BodyType::Static || _rigidB.type == BodyType::Static); 

        _motionA.linearVelocity = Add(_motionA.linearVelocity, Mul(impulse, _rigidA.massInverse));
        _motionB.linearVelocity = Subtract(_motionB.linearVelocity, Mul(impulse, _rigidB.massInverse));

        if (!supportLikeContact)
        {
            _motionA.angularVelocity = Add(
                _motionA.angularVelocity,
                ApplyInertiaInverse(Cross(cached.rA, impulse), _rigidA.inertiaTensorWorldInverse));

            _motionB.angularVelocity = Subtract(
                _motionB.angularVelocity,
                ApplyInertiaInverse(Cross(cached.rB, impulse), _rigidB.inertiaTensorWorldInverse));
        }
    }
}

void PhysicSystem::SolveFrictionImpulses(const ContactInfo& _contact,
    MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    const CachedContactPoint* _cache)
{
    for (int i = 0; i < _contact.pointCount; ++i)
    {
        const CachedContactPoint& cached = _cache[i];
        if (!cached.active)
            continue;

        if (cached.lastAppliedNormalImpulse <= 0.0f)
            continue;

        const XMFLOAT3 vA = VelocityAtPoint(_motionA, cached.rA);
        const XMFLOAT3 vB = VelocityAtPoint(_motionB, cached.rB);
        const XMFLOAT3 relativeVelocity = Subtract(vA, vB);

        ApplyFriction(_motionA, _motionB, _rigidA, _rigidB,
            cached.rA, cached.rB,
            _contact.normal,
            relativeVelocity,
            cached.lastAppliedNormalImpulse,
            _contact.pointCount);
    }
}

void PhysicSystem::ApplyFriction(MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    const XMFLOAT3& _rA, const XMFLOAT3& _rB,
    const XMFLOAT3& _normal, const XMFLOAT3& _relativeVelocityPreSolve,
    float _normalImpulse, int _pointCount)
{
    XMFLOAT3 tangentVelocity = Subtract(_relativeVelocityPreSolve, Mul(_normal, Dot(_relativeVelocityPreSolve, _normal)));
    const float tangentSpeed = sqrtf(LengthSq(tangentVelocity));
    if (tangentSpeed < 0.01f)
        return;

    const XMFLOAT3 tangent = Mul(tangentVelocity, 1.0f / tangentSpeed);

    const XMFLOAT3 vA = VelocityAtPoint(_motionA, _rA);
    const XMFLOAT3 vB = VelocityAtPoint(_motionB, _rB);
    const XMFLOAT3 relativeVelocityPost = Subtract(vA, vB);
    const float vt = Dot(relativeVelocityPost, tangent);

    const float effectiveMass =
        _rigidA.massInverse + _rigidB.massInverse +
        ComputeAngularMassTerm(_rA, tangent, _rigidA.inertiaTensorWorldInverse) +
        ComputeAngularMassTerm(_rB, tangent, _rigidB.inertiaTensorWorldInverse);

    if (effectiveMass <= 0.0f)
        return;

    const float jt = -vt / effectiveMass;

    const float muS = sqrtf(_rigidA.staticFriction * _rigidB.staticFriction);
    const float muD = sqrtf(_rigidA.dynamicFriction * _rigidB.dynamicFriction);

    const float pointScale = 1.0f / Max(1, _pointCount);
    const float maxStaticFriction = muS * _normalImpulse * pointScale;
    const float maxDynamicFriction = muD * _normalImpulse * pointScale;

    XMFLOAT3 frictionImpulse;
    if (fabsf(jt) <= maxStaticFriction)
    {
        frictionImpulse = Mul(tangent, jt);
    }
    else
    {
        const float sign = (jt < 0.0f) ? -1.0f : 1.0f;
        frictionImpulse = Mul(tangent, sign * maxDynamicFriction);
    }

    _motionA.linearVelocity = Add(_motionA.linearVelocity, Mul(frictionImpulse, _rigidA.massInverse));
    _motionA.angularVelocity = Add(
        _motionA.angularVelocity,
        ApplyInertiaInverse(Cross(_rA, frictionImpulse), _rigidA.inertiaTensorWorldInverse));

    _motionB.linearVelocity = Subtract(_motionB.linearVelocity, Mul(frictionImpulse, _rigidB.massInverse));
    _motionB.angularVelocity = Subtract(
        _motionB.angularVelocity,
        ApplyInertiaInverse(Cross(_rB, frictionImpulse), _rigidB.inertiaTensorWorldInverse));
}

void PhysicSystem::ApplyAngularPositionCorrection(EntityId _entity, RigidBodyComponent& _rigid, TransformComponent& _transform, const XMFLOAT3& _contactPoint, const XMFLOAT3& _normal, float _angularAmount) const
{
    if (_rigid.type != BodyType::Dynamic)
        return;

    if (!_rigid.allowRotation)
        return;

    if (_angularAmount <= 0.0f)
        return;

    const XMFLOAT3 center = GetBodyCenter(_entity);
    const XMFLOAT3 r = Subtract(_contactPoint, center);

    // Axe de correction angulaire : tend à réduire le biais créé par un contact excentré
    const XMFLOAT3 correctionTorqueAxis = Cross(_normal, r);
    const float lenSq = LengthSq(correctionTorqueAxis);
    if (lenSq <= 1e-10f)
        return;

    XMFLOAT3 angularDir = ApplyInertiaInverse(correctionTorqueAxis, _rigid.inertiaTensorWorldInverse);

    const float dirLenSq = LengthSq(angularDir);
    if (dirLenSq <= 1e-10f)
        return;

    const float dirLen = sqrtf(dirLenSq);
    angularDir.x /= dirLen;
    angularDir.y /= dirLen;
    angularDir.z /= dirLen;

    float angle = Min(_angularAmount, kMaxAngularPositionCorrection);

    XMFLOAT3 axisAngle =
    {
        angularDir.x * angle,
        angularDir.y * angle,
        angularDir.z * angle
    };

    RotateTransformByAxisAngle(_transform, axisAngle);
}

void PhysicSystem::ResolvePenetrations()
{
    for (CachedCollision& cachedCollision : m_collisionCache)
    {
        if (cachedCollision.collision == nullptr)
            continue;

        const CollisionResult& collision = *cachedCollision.collision;
        const ContactInfo& contact = cachedCollision.contact;

        if (!contact.hit || contact.pointCount <= 0)
            continue;

        RigidBodyComponent* rigidA = GetRigidBody(collision.entityA);
        RigidBodyComponent* rigidB = GetRigidBody(collision.entityB);
        if (rigidA == nullptr || rigidB == nullptr)
            continue;

        const float penetration = GetMaxPenetration(contact);
        const float correction = Max(0.0f, penetration - kPenetrationSlop) * kBaumgarteBeta;
        if (correction <= 0.0f)
            continue;

        const float totalInvMass = rigidA->massInverse + rigidB->massInverse;
        if (totalInvMass <= 0.0f)
            continue;

        const float moveA = correction * (rigidA->massInverse / totalInvMass);
        const float moveB = correction * (rigidB->massInverse / totalInvMass);

        TransformComponent& transformA = world->GetComponent<TransformComponent>(collision.entityA);
        TransformComponent& transformB = world->GetComponent<TransformComponent>(collision.entityB);

        const XMFLOAT3 contactPoint = ComputeAverageContactPoint(contact);

        if (rigidA->type == BodyType::Dynamic)
        {
            transformA.local.Move(Mul(contact.normal, moveA));
            ApplyAngularPositionCorrection(
                collision.entityA, *rigidA, transformA,
                contactPoint, contact.normal,
                moveA * kAngularPositionBeta);
        }

        if (rigidB->type == BodyType::Dynamic)
        {
            transformB.local.Move(Mul(contact.normal, -moveB));
            ApplyAngularPositionCorrection(
                collision.entityB, *rigidB, transformB,
                contactPoint, Inverse(contact.normal),
                moveB * kAngularPositionBeta);
        }
    }
}

void PhysicSystem::WakeSleepingPair(MotionComponent& _motionA, MotionComponent& _motionB,
    const RigidBodyComponent& _rigidA, const RigidBodyComponent& _rigidB)
{
    if (_rigidA.type == BodyType::Dynamic && _motionA.isSleeping &&
        _rigidB.type == BodyType::Dynamic && !_motionB.isSleeping)
    {
        _motionA.WakeUp();
    }

    if (_rigidB.type == BodyType::Dynamic && _motionB.isSleeping &&
        _rigidA.type == BodyType::Dynamic && !_motionA.isSleeping)
    {
        _motionB.WakeUp();
    }
}

XMFLOAT3 PhysicSystem::VelocityAtPoint(const MotionComponent& _motion, const XMFLOAT3& _r) const
{
    return Add(_motion.linearVelocity, Cross(_motion.angularVelocity, _r));
}

XMFLOAT3 PhysicSystem::ApplyInertiaInverse(const XMFLOAT3& _v, const float _tensor[9]) const
{
    return
    {
        _tensor[0] * _v.x + _tensor[1] * _v.y + _tensor[2] * _v.z,
        _tensor[3] * _v.x + _tensor[4] * _v.y + _tensor[5] * _v.z,
        _tensor[6] * _v.x + _tensor[7] * _v.y + _tensor[8] * _v.z
    };
}

XMFLOAT3 PhysicSystem::ComputeAverageContactPoint(const ContactInfo& _contact) const
{
    XMFLOAT3 p = { 0.f, 0.f, 0.f };

    if (_contact.pointCount <= 0)
        return p;

    for (int i = 0; i < _contact.pointCount; ++i)
    {
        p.x += _contact.points[i].position.x;
        p.y += _contact.points[i].position.y;
        p.z += _contact.points[i].position.z;
    }

    const float invCount = 1.0f / static_cast<float>(_contact.pointCount);
    p.x *= invCount;
    p.y *= invCount;
    p.z *= invCount;
    return p;
}

float PhysicSystem::ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _tensor[9]) const
{
    const XMFLOAT3 crossValue = Cross(_r, _axis);
    return Dot(Cross(ApplyInertiaInverse(crossValue, _tensor), _r), _axis);
}

void PhysicSystem::RotateTransformByAxisAngle(TransformComponent& _transform, const XMFLOAT3& _axisAngle) const
{
    XMVECTOR qCurrent = XMLoadFloat4(&_transform.local.GetRotation());
    XMVECTOR qDelta = XMQuaternionRotationRollPitchYaw(_axisAngle.x, _axisAngle.y, _axisAngle.z);
    XMVECTOR qNew = XMQuaternionNormalize(XMQuaternionMultiply(qDelta, qCurrent));

    XMFLOAT4 out;
    XMStoreFloat4(&out, qNew);
    _transform.local.SetRotationQuaternion(out);
}

void PhysicSystem::NotifyScripts()
{
    if (m_narrowPhase == nullptr)
        return;

    // Collisions classiques
    for (const CollisionResult& result : m_narrowPhase->GetResults())
    {
        world->NotifyScripts(result.entityA, &IScript::OnCollision, result.entityB);
        world->NotifyScripts(result.entityB, &IScript::OnCollision, result.entityA);
    }

    // Triggers
    for (const CollisionResult& result : m_narrowPhase->GetTriggerResults())
    {
        world->NotifyScripts(result.entityA, &IScript::OnTrigger, result.entityB);
        world->NotifyScripts(result.entityB, &IScript::OnTrigger, result.entityA);
    }
}

MotionComponent& PhysicSystem::GetMotion(EntityId _entity)
{
    if (world->HasComponent<MotionComponent>(_entity))
        return world->GetComponent<MotionComponent>(_entity);

    m_nullMotion = MotionComponent{};
    m_nullMotion.isSleeping = true;
    return m_nullMotion;
}

RigidBodyComponent* PhysicSystem::GetRigidBody(EntityId _entity)
{
    if (!world->HasComponent<RigidBodyComponent>(_entity))
        return nullptr;

    return &world->GetComponent<RigidBodyComponent>(_entity);
}

XMFLOAT3 PhysicSystem::GetBodyCenter(EntityId _entity) const
{
    if (world->HasComponent<ColliderComponent>(_entity))
        return world->GetComponent<ColliderComponent>(_entity).worldCenter;

    return world->GetComponent<TransformComponent>(_entity).world.GetPosition();
}

float PhysicSystem::GetMaxPenetration(const ContactInfo& _contact) const
{
    float maxPenetration = 0.0f;
    for (int i = 0; i < _contact.pointCount; ++i)
        maxPenetration = Max(maxPenetration, _contact.points[i].penetration);

    return maxPenetration;
}

bool PhysicSystem::ShouldReduceToCenterPoint(const ContactInfo& _contact) const
{
    if (!_contact.hit || _contact.pointCount < 2 || _contact.pointCount > 4)
        return false;

    // Contact de support quasi vertical
    const XMFLOAT3 up = { 0.f, 1.f, 0.f };
    if (fabsf(Dot(_contact.normal, up)) < 0.95f)
        return false;

    const float basePenetration = _contact.points[0].penetration;
    for (int i = 1; i < _contact.pointCount; ++i)
    {
        if (fabsf(_contact.points[i].penetration - basePenetration) > 0.01f)
            return false;
    }

    return true;
}

ContactInfo PhysicSystem::BuildReducedContact(const ContactInfo& _contact) const
{
    if (!ShouldReduceToCenterPoint(_contact))
        return _contact;

    ContactInfo reduced;
    reduced.hit = _contact.hit;
    reduced.normal = _contact.normal;
    reduced.pointCount = 1;

    XMFLOAT3 center = { 0.f, 0.f, 0.f };
    float maxPenetration = 0.f;

    for (int i = 0; i < _contact.pointCount; ++i)
    {
        center.x += _contact.points[i].position.x;
        center.y += _contact.points[i].position.y;
        center.z += _contact.points[i].position.z;

        maxPenetration = Max(maxPenetration, _contact.points[i].penetration);
    }

    const float invCount = 1.0f / static_cast<float>(_contact.pointCount);
    center.x *= invCount;
    center.y *= invCount;
    center.z *= invCount;

    reduced.points[0].position = center;
    reduced.points[0].penetration = maxPenetration;

    return reduced;
}