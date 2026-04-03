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

        // Résolution séquentielle : chaque point est traité l'un après l'autre.
        for (int i = 0; i < manifold.pointCount; ++i)
        {
            const ContactPoint& point = manifold.points[i];

            // rA, rB : vecteurs du centre de masse au point de contact.
            //   rA = Point i - GA
            //   rB = Point i - GB
            XMFLOAT3 rA = Subtract(point.position, GetCenter(manifold.a));
            XMFLOAT3 rB = Subtract(point.position, GetCenter(manifold.b));

            XMFLOAT3 vA = VelocityAtPoint(motionA, rA);
            XMFLOAT3 vB = VelocityAtPoint(motionB, rB);

            XMFLOAT3 vRel = Subtract(vB, vA);

            // vn = vitesse relative le long de la normale.
            // Si vn >= 0, les corps s'éloignent déjà : pas d'impulsion.

            float vn = Dot(vRel, manifold.normal);
            if (vn >= 0.0f)
                continue;

            // coefficient de restitution (e)
            float restitution = Min(rigidA->restitution, rigidB->restitution);
            if (fabsf(vn) < kRestitutionThreshold)
                restitution = 0.0f;


            //   Meff = 1/mA + 1/mB
            //        + (IA^-1 * (rA x n) x rA) ° n
            //        + (IB^-1 * (rB x n) x rB) ° n
            float effectiveMass = rigidA->massInverse + rigidB->massInverse
                + AngularMassTerm(rA, manifold.normal, rigidA->inertiaTensorWorldInverse)
                + AngularMassTerm(rB, manifold.normal, rigidB->inertiaTensorWorldInverse);

            if (effectiveMass <= 0.0f)
                continue;

            float bias = kBeta * invDt * Max(0.0f, point.penetration - kPenetrationSlop);

            // j : scalaire de l'impulsion.
            // j = -(1 + e) * vn / Meff
            // Le signe négatif inverse la composante de rapprochement.
            float j = (-(1.0f + restitution) * vn + bias) / effectiveMass;

            // J : vecteur d'impulsion, orienté selon la normale de contact.
            // J = j * n
            XMFLOAT3 J = Mul(manifold.normal, j);

            // Application immédiate de l'impulsion sur les vitesses.
            // Translation :
            //   vGA' = vGA - J / mA     (A reçoit l'impulsion en sens inverse)
            //   vGB' = vGB + J / mB     (B reçoit l'impulsion dans le sens de n)
            // Rotation :
            //   omegaA' = omegaA - IA^-1 * (rA x J)
            //   omegaB' = omegaB + IB^-1 * (rB x J)

            motionA.linearVelocity = Subtract(motionA.linearVelocity, Mul(J, rigidA->massInverse));
            motionA.angularVelocity = Subtract(motionA.angularVelocity,
                ApplyInertiaInverse(Cross(rA, J), rigidA->inertiaTensorWorldInverse));

            motionB.linearVelocity = Add(motionB.linearVelocity, Mul(J, rigidB->massInverse));
            motionB.angularVelocity = Add(motionB.angularVelocity,
                ApplyInertiaInverse(Cross(rB, J), rigidB->inertiaTensorWorldInverse));
        }

		motionA.linearVelocity = Snap(motionA.linearVelocity, kLinearSnapThreshold);
		motionA.angularVelocity = Snap(motionA.angularVelocity, kAngularSnapThreshold);
		motionB.linearVelocity = Snap(motionB.linearVelocity, kLinearSnapThreshold);
		motionB.angularVelocity = Snap(motionB.angularVelocity, kAngularSnapThreshold);
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

XMFLOAT3 PhysicSystem::Snap(const XMFLOAT3& _v, float _threshold) const
{
    return
    {
        (fabsf(_v.x) < _threshold) ? 0.0f : _v.x,
        (fabsf(_v.y) < _threshold) ? 0.0f : _v.y,
        (fabsf(_v.z) < _threshold) ? 0.0f : _v.z
    };
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
    return world->GetComponent<TransformComponent>(_e).world.GetPosition();
}