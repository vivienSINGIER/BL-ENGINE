#include "PhysicSystem.h"
#include "../ECS/World.h"

void PhysicSystem::Update(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    const auto& collisions = m_narrowPhase->GetResults();
    if (collisions.empty())
        return;

    for (int i = 0; i < kVelocityIterations; ++i)
        ResolveVelocities(_dt);

    for (int i = 0; i < kPositionIterations; ++i)
        ResolvePenetrations();
}

void PhysicSystem::ResolveVelocities(float _dt)
{
    if (_dt <= 0.0f)
        return;

    const float invDt = 1.0f / _dt;
    const auto& collisions = m_narrowPhase->GetResults();

    for (const CollisionResult& collision : collisions)
        ResolveCollisionVelocities(collision, invDt);
}

void PhysicSystem::ResolveCollisionVelocities(const CollisionResult& _collision, float _invDt)
{
    const ContactInfo& contact = _collision.contact;
    if (!contact.hit || contact.pointCount <= 0)
        return;

    RigidBodyComponent* rigidA = GetRigidBody(_collision.entityA);
    RigidBodyComponent* rigidB = GetRigidBody(_collision.entityB);
    if (rigidA == nullptr || rigidB == nullptr)
        return;

    if (rigidA->massInverse + rigidB->massInverse <= 0.0f)
        return;

    MotionComponent& motionA = GetMotion(_collision.entityA);
    MotionComponent& motionB = GetMotion(_collision.entityB);

    if (motionA.isSleeping && motionB.isSleeping)
        return;

    WakeSleepingPair(motionA, motionB, *rigidA, *rigidB);

    const XMFLOAT3 centerA = GetBodyCenter(_collision.entityA);
    const XMFLOAT3 centerB = GetBodyCenter(_collision.entityB);

    CachedContactPoint cache[kMaxContactPoints];
    BuildContactPointCache(contact, motionA, motionB, *rigidA, *rigidB, centerA, centerB, _invDt, cache);

    SolveNormalImpulses(contact, motionA, motionB, *rigidA, *rigidB, cache);
    SolveFrictionImpulses(contact, motionA, motionB, *rigidA, *rigidB, cache);

    motionA.linearVelocity = Snap(motionA.linearVelocity, kLinearSnapThreshold);
    motionA.angularVelocity = Snap(motionA.angularVelocity, kAngularSnapThreshold);
    motionB.linearVelocity = Snap(motionB.linearVelocity, kLinearSnapThreshold);
    motionB.angularVelocity = Snap(motionB.angularVelocity, kAngularSnapThreshold);
}

void PhysicSystem::BuildContactPointCache(const ContactInfo& _contact,
    MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    const XMFLOAT3& _centerA, const XMFLOAT3& _centerB,
    float _invDt,
    CachedContactPoint* _cache)
{
    for (int i = 0; i < _contact.pointCount; ++i)
    {
        CachedContactPoint& cached = _cache[i];
        cached = CachedContactPoint{};

        const ContactPoint& point = _contact.points[i];

        cached.rA = Subtract(point.position, _centerA);
        cached.rB = Subtract(point.position, _centerB);

        const XMFLOAT3 vA = VelocityAtPoint(_motionA, cached.rA);
        const XMFLOAT3 vB = VelocityAtPoint(_motionB, cached.rB);

        // Convention : vitesse relative de A par rapport à B.
        cached.relativeVelocity = Subtract(vA, vB);

        const float vn = Dot(cached.relativeVelocity, _contact.normal);
        if (vn >= 0.0f)
            continue;

        float restitution = Min(_rigidA.restitution, _rigidB.restitution);
        if (fabsf(vn) < kRestitutionThreshold)
            restitution = 0.0f;

        const float effectiveMass =
            _rigidA.massInverse + _rigidB.massInverse +
            ComputeAngularMassTerm(cached.rA, _contact.normal, _rigidA.inertiaTensorWorldInverse) +
            ComputeAngularMassTerm(cached.rB, _contact.normal, _rigidB.inertiaTensorWorldInverse);

        if (effectiveMass <= 0.0f)
            continue;

        const float bias = kBaumgarteBeta * _invDt * Max(0.0f, point.penetration - kPenetrationSlop);

        cached.normalImpulse = -(1.0f + restitution) * vn / effectiveMass;
        cached.biasImpulse = bias / effectiveMass;
        cached.active = true;
    }
}

void PhysicSystem::SolveNormalImpulses(const ContactInfo& _contact,
    MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    const CachedContactPoint* _cache)
{
    for (int i = 0; i < _contact.pointCount; ++i)
    {
        const CachedContactPoint& cached = _cache[i];
        if (!cached.active)
            continue;

        const XMFLOAT3 impulsePhysics = Mul(_contact.normal, cached.normalImpulse);
        const XMFLOAT3 impulseBias = Mul(_contact.normal, cached.biasImpulse);
        const XMFLOAT3 totalImpulse = Add(impulsePhysics, impulseBias);

        _motionA.linearVelocity = Add(_motionA.linearVelocity, Mul(totalImpulse, _rigidA.massInverse));
        _motionB.linearVelocity = Subtract(_motionB.linearVelocity, Mul(totalImpulse, _rigidB.massInverse));

        _motionA.angularVelocity = Add(
            _motionA.angularVelocity,
            ApplyInertiaInverse(Cross(cached.rA, impulsePhysics), _rigidA.inertiaTensorWorldInverse));

        _motionB.angularVelocity = Subtract(
            _motionB.angularVelocity,
            ApplyInertiaInverse(Cross(cached.rB, impulsePhysics), _rigidB.inertiaTensorWorldInverse));
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

        ApplyFriction(_motionA, _motionB, _rigidA, _rigidB,
            cached.rA, cached.rB,
            _contact.normal,
            cached.relativeVelocity,
            cached.normalImpulse,
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

void PhysicSystem::ResolvePenetrations()
{
    const auto& collisions = m_narrowPhase->GetResults();

    for (const CollisionResult& collision : collisions)
    {
        const ContactInfo& contact = collision.contact;
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

        // normal = de B vers A
        if (rigidA->type == BodyType::Dynamic)
            transformA.local.Move(Mul(contact.normal, moveA));

        if (rigidB->type == BodyType::Dynamic)
            transformB.local.Move(Mul(contact.normal, -moveB));
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

float PhysicSystem::ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _tensor[9]) const
{
    const XMFLOAT3 crossValue = Cross(_r, _axis);
    return Dot(Cross(ApplyInertiaInverse(crossValue, _tensor), _r), _axis);
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
