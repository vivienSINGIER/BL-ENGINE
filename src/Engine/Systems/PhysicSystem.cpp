#include "PhysicSystem.h"
#include "../ECS/World.h"

void PhysicSystem::Update(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    const auto& results = m_narrowPhase->GetResults();
    if (results.empty())
        return;


    const int velocityIterations = 8;
    for (int i = 0; i < velocityIterations; ++i)
        ResolveVelocities(_dt);

    const int positionIterations = 3;
    for (int i = 0; i < positionIterations; ++i)
        ResolvePenetrations();
}

void PhysicSystem::ResolveVelocities(float _dt)
{
    const float invDt = (_dt > 0.0f) ? 1.0f / _dt : 0.0f;
    const auto& results = m_narrowPhase->GetResults();

    for (const CollisionResult& collision : results)
    {
        const ContactInfo& contact = collision.contact;

        if (!contact.hit || contact.pointCount <= 0)
            continue;

        RigidBodyComponent* rigidA = GetRigid(collision.entityA);
        RigidBodyComponent* rigidB = GetRigid(collision.entityB);

        if (rigidA == nullptr || rigidB == nullptr)
            continue;

        if (rigidA->massInverse + rigidB->massInverse <= 0.0f)
            continue;

        MotionComponent& motionA = GetMotion(collision.entityA);
        MotionComponent& motionB = GetMotion(collision.entityB);

        if (motionA.isSleeping && motionB.isSleeping)
            continue;

        UpdateSleepState(motionA, motionB, rigidA, rigidB);

        struct PointCache
        {
            XMFLOAT3 rA, rB;
            XMFLOAT3 vRelPre;
            float    jPhysics;
            float    jBias;
            bool     active;
        };

        PointCache cache[kMaxContactPoints];

        XMFLOAT3 centerA = GetCenter(collision.entityA);
        XMFLOAT3 centerB = GetCenter(collision.entityB);

        TransformComponent& transformA = world->GetComponent<TransformComponent>(collision.entityA);
        TransformComponent& transformB = world->GetComponent<TransformComponent>(collision.entityB);

        const ColliderComponent& colliderA = world->GetComponent<ColliderComponent>(collision.entityA);
        const ColliderComponent& colliderB = world->GetComponent<ColliderComponent>(collision.entityB);

        for (int i = 0; i < contact.pointCount; ++i)
        {
            PointCache& pc = cache[i];
            pc.active = false;

            const ContactPoint& point = contact.points[i];

            pc.rA = Subtract(point.position, centerA);
            pc.rB = Subtract(point.position, centerB);

            XMFLOAT3 vA = VelocityAtPoint(motionA, pc.rA);
            XMFLOAT3 vB = VelocityAtPoint(motionB, pc.rB);

            pc.vRelPre = Subtract(vA, vB);

            float vn = Dot(pc.vRelPre, contact.normal);
            if (vn >= 0.0f)
                continue;

            float restitution = Min(rigidA->restitution, rigidB->restitution);
            if (fabsf(vn) < kRestitutionThreshold)
                restitution = 0.0f;

            float effectiveMass =
                rigidA->massInverse + rigidB->massInverse +
                AngularMassTerm(pc.rA, contact.normal, rigidA->inertiaTensorWorldInverse) +
                AngularMassTerm(pc.rB, contact.normal, rigidB->inertiaTensorWorldInverse);

            if (effectiveMass <= 0.0f)
                continue;

            float bias = kBeta * invDt * Max(0.0f, point.penetration - kPenetrationSlop);

            pc.jPhysics = -(1.0f + restitution) * vn / effectiveMass;
            pc.jBias = bias / effectiveMass;
            pc.active = true;

            printf("  rA%d = (%f, %f, %f), j = %f\n",
                i, pc.rA.x, pc.rA.y, pc.rA.z, pc.jPhysics);
            printf("  rB%d = (%f, %f, %f)\n", 
                i, pc.rB.x, pc.rB.y, pc.rB.z);
        }

        // impulsion normale
        for (int i = 0; i < contact.pointCount; ++i)
        {
            const PointCache& pc = cache[i];
            if (!pc.active)
                continue;

            XMFLOAT3 JPhysics = Mul(contact.normal, pc.jPhysics);
            XMFLOAT3 JBias = Mul(contact.normal, pc.jBias);
            XMFLOAT3 JTotal = Add(JPhysics, JBias);

            motionA.linearVelocity = Add(motionA.linearVelocity, Mul(JTotal, rigidA->massInverse));
            motionB.linearVelocity = Subtract(motionB.linearVelocity, Mul(JTotal, rigidB->massInverse));

            motionA.angularVelocity = Add(
                motionA.angularVelocity,
                ApplyInertiaInverse(Cross(pc.rA, JPhysics), rigidA->inertiaTensorWorldInverse));

            motionB.angularVelocity = Subtract(
                motionB.angularVelocity,
                ApplyInertiaInverse(Cross(pc.rB, JPhysics), rigidB->inertiaTensorWorldInverse));
        }

        // friction
        for (int i = 0; i < contact.pointCount; ++i)
        {
            const PointCache& pc = cache[i];
            if (!pc.active)
                continue;

            ApplyFriction(
                motionA, motionB,
                rigidA, rigidB,
                pc.rA, pc.rB,
                contact.normal,
                pc.vRelPre,
                pc.jPhysics,
                contact.pointCount);
        }

        motionA.linearVelocity = Snap(motionA.linearVelocity, kLinearSnapThreshold);
        motionA.angularVelocity = Snap(motionA.angularVelocity, kAngularSnapThreshold);
        motionB.linearVelocity = Snap(motionB.linearVelocity, kLinearSnapThreshold);
        motionB.angularVelocity = Snap(motionB.angularVelocity, kAngularSnapThreshold);
    }
}

void PhysicSystem::UpdateSleepState(MotionComponent& _motionA, MotionComponent& _motionB, RigidBodyComponent* _rigidA, RigidBodyComponent* _rigidB)
{
    if (_rigidA->type == BodyType::Dynamic && _motionA.isSleeping &&
        _rigidB->type == BodyType::Dynamic && !_motionB.isSleeping)
    {
        _motionA.WakeUp();
    }

    if (_rigidB->type == BodyType::Dynamic && _motionB.isSleeping &&
        _rigidA->type == BodyType::Dynamic && !_motionA.isSleeping)
    {
        _motionB.WakeUp();
    }
}

void PhysicSystem::ApplyFriction(
    MotionComponent& _motionA, MotionComponent& _motionB,
    RigidBodyComponent* _rigidA, RigidBodyComponent* _rigidB,
    const XMFLOAT3& _rA, const XMFLOAT3& _rB,
    const XMFLOAT3& _normal, const XMFLOAT3& _vRelPre,
    float _j, int _pointCount)
{
    XMFLOAT3 vTangentPre = Subtract(_vRelPre, Mul(_normal, Dot(_vRelPre, _normal)));
    float vTangentPreLen = sqrtf(LengthSq(vTangentPre));

    if (vTangentPreLen < 0.01f)
        return;

    XMFLOAT3 t = Mul(vTangentPre, 1.0f / vTangentPreLen);

    XMFLOAT3 vA = VelocityAtPoint(_motionA, _rA);
    XMFLOAT3 vB = VelocityAtPoint(_motionB, _rB);
    XMFLOAT3 vRelPost = Subtract(vA, vB);
    float vt = Dot(vRelPost, t);

    float meffT =
        _rigidA->massInverse + _rigidB->massInverse +
        AngularMassTerm(_rA, t, _rigidA->inertiaTensorWorldInverse) +
        AngularMassTerm(_rB, t, _rigidB->inertiaTensorWorldInverse);

    if (meffT <= 0.0f)
        return;

    float jt = -vt / meffT;

    float muS = sqrtf(_rigidA->staticFriction * _rigidB->staticFriction);
    float muD = sqrtf(_rigidA->dynamicFriction * _rigidB->dynamicFriction);

    float pointScale = 1.0f / Max(1, _pointCount);
    float maxStatic = muS * _j * pointScale;
    float maxDynamic = muD * _j * pointScale;

    XMFLOAT3 Jt;
    if (fabsf(jt) <= maxStatic)
    {
        Jt = Mul(t, jt);
    }
    else
    {
        float sign = (jt < 0.0f) ? -1.0f : 1.0f;
        Jt = Mul(t, sign * maxDynamic);
    }

    _motionA.linearVelocity = Add(_motionA.linearVelocity, Mul(Jt, _rigidA->massInverse));
    _motionA.angularVelocity = Add(
        _motionA.angularVelocity,
        ApplyInertiaInverse(Cross(_rA, Jt), _rigidA->inertiaTensorWorldInverse));

    _motionB.linearVelocity = Subtract(_motionB.linearVelocity, Mul(Jt, _rigidB->massInverse));
    _motionB.angularVelocity = Subtract(
        _motionB.angularVelocity,
        ApplyInertiaInverse(Cross(_rB, Jt), _rigidB->inertiaTensorWorldInverse));
}

void PhysicSystem::ResolvePenetrations()
{
    const auto& results = m_narrowPhase->GetResults();

    for (const CollisionResult& collision : results)
    {
        const ContactInfo& contact = collision.contact;

        if (!contact.hit || contact.pointCount <= 0)
            continue;

        RigidBodyComponent* rA = GetRigid(collision.entityA);
        RigidBodyComponent* rB = GetRigid(collision.entityB);

        if (rA == nullptr || rB == nullptr)
            continue;

        float penetration = GetMaxPenetration(contact);
        float correction = Max(0.0f, penetration - kPenetrationSlop) * kBeta;
        if (correction <= 0.0f)
            continue;

        float totalInvMass = rA->massInverse + rB->massInverse;
        if (totalInvMass <= 0.0f)
            continue;

        float moveA = correction * (rA->massInverse / totalInvMass);
        float moveB = correction * (rB->massInverse / totalInvMass);

        TransformComponent& tA = world->GetComponent<TransformComponent>(collision.entityA);
        TransformComponent& tB = world->GetComponent<TransformComponent>(collision.entityB);

        // normal = de B vers A
        // donc A bouge dans +normal, B dans -normal
        if (rA->type == BodyType::Dynamic)
            tA.local.Move(Mul(contact.normal, moveA));

        if (rB->type == BodyType::Dynamic)
            tB.local.Move(Mul(contact.normal, -moveB));
    }
}

XMFLOAT3 PhysicSystem::VelocityAtPoint(MotionComponent& _motion, const XMFLOAT3& _r) const
{
    return Add(_motion.linearVelocity, Cross(_motion.angularVelocity, _r));
}

XMFLOAT3 PhysicSystem::ApplyInertiaInverse(const XMFLOAT3& _v, const float _t[9]) const
{
    return
    {
        _t[0] * _v.x + _t[1] * _v.y + _t[2] * _v.z,
        _t[3] * _v.x + _t[4] * _v.y + _t[5] * _v.z,
        _t[6] * _v.x + _t[7] * _v.y + _t[8] * _v.z
    };
}

float PhysicSystem::AngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _t[9]) const
{
    XMFLOAT3 rxn = Cross(_r, _axis);
    return Dot(Cross(ApplyInertiaInverse(rxn, _t), _r), _axis);
}

MotionComponent& PhysicSystem::GetMotion(EntityId _e)
{
    if (world->HasComponent<MotionComponent>(_e))
        return world->GetComponent<MotionComponent>(_e);

    m_nullMotion = MotionComponent{};
    m_nullMotion.isSleeping = true;
    return m_nullMotion;
}

RigidBodyComponent* PhysicSystem::GetRigid(EntityId _e)
{
    if (!world->HasComponent<RigidBodyComponent>(_e))
        return nullptr;

    return &world->GetComponent<RigidBodyComponent>(_e);
}

XMFLOAT3 PhysicSystem::GetCenter(EntityId _e) const
{
    if (world->HasComponent<ColliderComponent>(_e))
        return world->GetComponent<ColliderComponent>(_e).worldCenter;

    return world->GetComponent<TransformComponent>(_e).world.GetPosition();
}

float PhysicSystem::GetMaxPenetration(const ContactInfo& _contact) const
{
    float maxPen = 0.0f;
    for (int i = 0; i < _contact.pointCount; ++i)
        maxPen = Max(maxPen, _contact.points[i].penetration);
    return maxPen;
}