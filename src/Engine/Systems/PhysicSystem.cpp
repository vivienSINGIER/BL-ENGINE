#include "PhysicSystem.h"
#include "../ECS/World.h"

void PhysicSystem::Update(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    auto& manifolds = m_narrowPhase->GetManifoldCache().manifolds;
    if (manifolds.empty())
        return;

    ResolveVelocities();
    ResolvePenetrations();
}

void PhysicSystem::ResolveVelocities()
{
    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (manifold.isTrigger)
            continue;

        RigidBodyComponent* rA = GetRigid(manifold.a);
        RigidBodyComponent* rB = GetRigid(manifold.b);
        if (!rA || !rB)
            continue;

        if (rA->massInverse + rB->massInverse <= 0.0f)
            continue;

        MotionComponent& mA = GetMotion(manifold.a);
        MotionComponent& mB = GetMotion(manifold.b);

        for (int i = 0; i < manifold.pointCount; ++i)
        {
            const XMFLOAT3& point = manifold.points[i].position;

            // Vecteurs du centre de masse au point de contact.
            XMFLOAT3 vecA = Subtract(point, GetCenter(manifold.a));
            XMFLOAT3 vecB = Subtract(point, GetCenter(manifold.b));

            // Vitesse au point de contact pour chaque corps.
            XMFLOAT3 vA = VelocityAtPoint(mA, vecA);
            XMFLOAT3 vB = VelocityAtPoint(mB, vecB);

            // Vitesse relative au point (B - A).
            XMFLOAT3 relVel = Subtract(vB, vA);

            // Projection sur la normale de contact.
            float vn = Dot(relVel, manifold.normal);

            // Pas d'impulsion si les corps s'éloignent déjà.
            if (vn >= 0.0f)
                continue;

            // Restitution — annulée pour les contacts quasi-statiques
            // (évite le micro-rebond à l'arrêt).
            float restitution = Min(rA->restitution, rB->restitution);
            if (fabsf(vn) < kRestitutionThreshold)
                restitution = 0.0f;

            // Masse effective au point de contact.
            float effectiveMass = rA->massInverse + rB->massInverse +
                AngularMassTerm(vecA, manifold.normal, rA->inertiaTensorWorldInverse) +
                AngularMassTerm(vecB, manifold.normal, rB->inertiaTensorWorldInverse);

            if (effectiveMass <= 0.0f)
                continue;

            // Calcul de l'impulsion scalaire, répartie sur tous les points.
            float lambda = -(1.0f + restitution) * vn / (effectiveMass * manifold.pointCount);

            // Application de l'impulsion.
            XMFLOAT3 impulse = Mul(manifold.normal, lambda);
            ApplyImpulse(mA, *rA, impulse, vecA, -1.0f);
            ApplyImpulse(mB, *rB, impulse, vecB, +1.0f);
        }
    }
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
        // A doit se déplacer dans -normal, B dans +normal.
        if (rA->type == BodyType::Dynamic)
            tA.local.Move(Mul(manifold.normal, -moveA));

        if (rB->type == BodyType::Dynamic)
            tB.local.Move(Mul(manifold.normal, moveB));
    }
}

XMFLOAT3 PhysicSystem::VelocityAtPoint(MotionComponent& _motion, const XMFLOAT3& _r) const
{
    // v_point = linearVelocity + angularVelocity × r
    return Add(_motion.linearVelocity, Cross(_motion.angularVelocity, _r));
}

void PhysicSystem::ApplyImpulse(MotionComponent& _motion, RigidBodyComponent& _rigid, const XMFLOAT3& _impulse, const XMFLOAT3& _r, float _sign) const
{
    if (_rigid.type == BodyType::Static || _rigid.type == BodyType::Kinematic)
        return;

    _motion.linearVelocity = Add(_motion.linearVelocity,
        Mul(_impulse, _rigid.massInverse * _sign));

    if (_rigid.allowRotation)
    {
		XMFLOAT3 vec = Cross(_r, _impulse);
        XMFLOAT3 dw = ApplyInertiaInverse(vec, _rigid.inertiaTensorWorldInverse);
        _motion.angularVelocity = Add(_motion.angularVelocity, Mul(dw, _sign));
    }
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
    return world->GetComponent<TransformComponent>(_e).local.GetPosition();
}