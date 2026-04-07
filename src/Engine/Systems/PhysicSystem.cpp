#include "PhysicSystem.h"
#include "../ECS/World.h"

void PhysicSystem::Update(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    auto& manifolds = m_narrowPhase->GetManifoldCache().manifolds;
    if (manifolds.empty())
        return;

    ResolveVelocities(_dt);
    ResolvePenetrations();
}

void PhysicSystem::ResolveVelocities(float _dt)
{
    const float invDt = (_dt > 0.0f) ? 1.0f / _dt : 0.0f;

    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (manifold.isTrigger)
            continue;

        RigidBodyComponent* rigidA = GetRigid(manifold.a);
        RigidBodyComponent* rigidB = GetRigid(manifold.b);

        // Si les deux corps ont une masse infinie (statiques), rien à résoudre.
        if (rigidA->massInverse + rigidB->massInverse <= 0.0f)
            continue;

        MotionComponent& motionA = GetMotion(manifold.a);
        MotionComponent& motionB = GetMotion(manifold.b);

        if (motionA.isSleeping && motionB.isSleeping)
            continue;

        UpdateSleepState(motionA, motionB, rigidA, rigidB);

        struct PointCache
        {
            XMFLOAT3 rA, rB;
            XMFLOAT3 vRelPre;   // vRel avant toute impulsion (pour la friction)
            float    jPhysics;  // impulsion physique pure (sans biais)
            float    jBias;     // correction positionnelle (translation uniquement)
            bool     active;    // false si vn >= 0
        };

        PointCache cache[ContactManifold::kMaxPoints];

        XMFLOAT3 centerA = GetCenter(manifold.a);
        XMFLOAT3 centerB = GetCenter(manifold.b);

        for (int i = 0; i < manifold.pointCount; ++i)
        {
            PointCache& pc = cache[i];
            pc.active = false;

            const ContactPoint& point = manifold.points[i];

            pc.rA = Subtract(point.position, centerA);
            pc.rB = Subtract(point.position, centerB);

            XMFLOAT3 vA = VelocityAtPoint(motionA, pc.rA);
            XMFLOAT3 vB = VelocityAtPoint(motionB, pc.rB);
            pc.vRelPre = Subtract(vB, vA);

            float vn = Dot(pc.vRelPre, manifold.normal);
            if (vn >= 0.0f)
                continue;

            float restitution = Min(rigidA->restitution, rigidB->restitution);
            if (fabsf(vn) < kRestitutionThreshold)
                restitution = 0.0f;

            float effectiveMass = rigidA->massInverse + rigidB->massInverse
                + AngularMassTerm(pc.rA, manifold.normal, rigidA->inertiaTensorWorldInverse)
                + AngularMassTerm(pc.rB, manifold.normal, rigidB->inertiaTensorWorldInverse);

            if (effectiveMass <= 0.0f)
                continue;

            float bias = kBeta * invDt * Max(0.0f, point.penetration - kPenetrationSlop);
            pc.jPhysics = -(1.0f + restitution) * vn / effectiveMass;
            pc.jBias = bias / effectiveMass;
            pc.active = true;
        }

        for (int i = 0; i < manifold.pointCount; ++i)
        {
            const PointCache& pc = cache[i];
            if (!pc.active) continue;

            XMFLOAT3 JPhysics = Mul(manifold.normal, pc.jPhysics);
            XMFLOAT3 JBias = Mul(manifold.normal, pc.jBias);

            motionA.linearVelocity = Subtract(motionA.linearVelocity,
                Mul(Add(JPhysics, JBias), rigidA->massInverse));
            motionB.linearVelocity = Add(motionB.linearVelocity,
                Mul(Add(JPhysics, JBias), rigidB->massInverse));

            motionA.angularVelocity = Subtract(motionA.angularVelocity,
                ApplyInertiaInverse(Cross(pc.rA, JPhysics), rigidA->inertiaTensorWorldInverse));
            motionB.angularVelocity = Add(motionB.angularVelocity,
                ApplyInertiaInverse(Cross(pc.rB, JPhysics), rigidB->inertiaTensorWorldInverse));
        }

        for (int i = 0; i < manifold.pointCount; ++i)
        {
            const PointCache& pc = cache[i];
            if (!pc.active) continue;

            ApplyFriction(motionA, motionB, rigidA, rigidB,
                pc.rA, pc.rB, manifold.normal, pc.vRelPre, pc.jPhysics);
        }

        motionA.linearVelocity = Snap(motionA.linearVelocity, kLinearSnapThreshold);
        motionA.angularVelocity = Snap(motionA.angularVelocity, kAngularSnapThreshold);
        motionB.linearVelocity = Snap(motionB.linearVelocity, kLinearSnapThreshold);
        motionB.angularVelocity = Snap(motionB.angularVelocity, kAngularSnapThreshold);
    }
}

void PhysicSystem::UpdateSleepState(MotionComponent& _motionA, MotionComponent& _motionB, RigidBodyComponent* _rigidA, RigidBodyComponent* _rigidB)
{
    if (_rigidA->type == BodyType::Dynamic && _motionA.isSleeping && _rigidB->type == BodyType::Dynamic && !_motionB.isSleeping)
        _motionA.WakeUp();

    if (_rigidB->type == BodyType::Dynamic && _motionB.isSleeping && _rigidA->type == BodyType::Dynamic && !_motionA.isSleeping)
        _motionB.WakeUp();
}

void PhysicSystem::ApplyFriction(MotionComponent& _motionA, MotionComponent& _motionB, RigidBodyComponent* _rigidA, RigidBodyComponent* _rigidB,
    const XMFLOAT3& _rA, const XMFLOAT3& _rB, const XMFLOAT3& _normal, const XMFLOAT3& _vRelPre, float _j)
{
    // Friction de Coulomb — séparation direction / magnitude.
    //
    // DIRECTION t : depuis _vRelPre (état initial, commun à tous les points).
    // MAGNITUDE   : depuis vRel post-impulsion normale.

    XMFLOAT3 vTangentPre = Subtract(_vRelPre, Mul(_normal, Dot(_vRelPre, _normal)));
    float    vTangentPreLen = sqrtf(NormSquared(vTangentPre));

    if (vTangentPreLen < 0.01f)
        return;

    // t : direction stable, identique pour tous les points du manifold.
    XMFLOAT3 t = Mul(vTangentPre, 1.0f / vTangentPreLen);

    // Magnitude depuis vRel post-impulsion normale.
    XMFLOAT3 vA = VelocityAtPoint(_motionA, _rA);
    XMFLOAT3 vB = VelocityAtPoint(_motionB, _rB);
    XMFLOAT3 vRelPost = Subtract(vB, vA);
    float    vt = Dot(vRelPost, t);

    // Masse effective dans la direction tangentielle.
    float Meff_t = _rigidA->massInverse + _rigidB->massInverse
        + AngularMassTerm(_rA, t, _rigidA->inertiaTensorWorldInverse)
        + AngularMassTerm(_rB, t, _rigidB->inertiaTensorWorldInverse);

    if (Meff_t <= 0.0f)
        return;

    float jt = -vt / Meff_t;

    // Coefficients de friction combinés (moyenne géométrique).
    float muS = sqrtf(_rigidA->staticFriction * _rigidB->staticFriction);
    float muD = sqrtf(_rigidA->dynamicFriction * _rigidB->dynamicFriction);

    // Loi de Coulomb — borné par jPhysics (sans biais).
    XMFLOAT3 Jt;
    if (fabsf(jt) <= muS * _j)
        Jt = Mul(t, jt);         // Statique : annule le glissement.
    else
        Jt = Mul(t, -muD * _j); // Dynamique : plafonne à mu_d * j.

    _motionA.linearVelocity = Subtract(_motionA.linearVelocity, Mul(Jt, _rigidA->massInverse));
    _motionA.angularVelocity = Subtract(_motionA.angularVelocity,
        ApplyInertiaInverse(Cross(_rA, Jt), _rigidA->inertiaTensorWorldInverse));

    _motionB.linearVelocity = Add(_motionB.linearVelocity, Mul(Jt, _rigidB->massInverse));
    _motionB.angularVelocity = Add(_motionB.angularVelocity,
        ApplyInertiaInverse(Cross(_rB, Jt), _rigidB->inertiaTensorWorldInverse));
}

void PhysicSystem::ResolvePenetrations()
{
    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (manifold.isTrigger)
            continue;

        RigidBodyComponent* rA = GetRigid(manifold.a);
        RigidBodyComponent* rB = GetRigid(manifold.b);

        float correction = Max(0.0f, manifold.penetration - kPenetrationSlop) * kBeta;
        if (correction <= 0.0f)
            continue;

        float totalInvMass = rA->massInverse + rB->massInverse;
        if (totalInvMass <= 0.0f)
            continue;

        float moveA = correction * (rA->massInverse / totalInvMass);
        float moveB = correction * (rB->massInverse / totalInvMass);

        TransformComponent& tA = world->GetComponent<TransformComponent>(manifold.a);
        TransformComponent& tB = world->GetComponent<TransformComponent>(manifold.b);

        // La normale pointe de A vers B.
        // A se déplace dans -normal, B dans +normal.
        if (rA->type == BodyType::Dynamic)
            tA.local.Move(Mul(manifold.normal, -moveA));

        if (rB->type == BodyType::Dynamic)
            tB.local.Move(Mul(manifold.normal, moveB));
    }
}

XMFLOAT3 PhysicSystem::VelocityAtPoint(MotionComponent& _motion, const XMFLOAT3& _r) const
{
    // v_point = v_G + omega x r
    // v_G   : vitesse linéaire du centre de masse
    // omega : vitesse angulaire du corps
    // r     : vecteur du centre de masse au point
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
    return world->GetComponent<TransformComponent>(_e).world.GetPosition();
}