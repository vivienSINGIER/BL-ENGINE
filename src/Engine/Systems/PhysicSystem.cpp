#include "PhysicSystem.h"
#include "../ECS/World.h"
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// OnEndUpdate — pipeline principal
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::OnEndUpdate(float _dt)
{
    if (m_narrowPhase == nullptr)
        return;

    auto& manifolds = m_narrowPhase->GetManifoldCache().manifolds;
    if (manifolds.empty())
        return;

	ResetPseudoVelocities();

    // Les tenseurs monde doivent être frais avant le warm start.
    UpdateWorldInertias();
    WarmStart();
    SolveConstraints(kSolverIterations);
	SolvePositionConstraints(kSolverIterations);
    WakeBodies();
}

// ─────────────────────────────────────────────────────────────────────────────
// Mise à jour des tenseurs monde
//
//  I_world_inv = R * I_body_inv * R^T
//  Calculé ici une seule fois par frame pour tous les corps Dynamic qui
//  ont des contacts actifs, plutôt que dans l'intégrateur.
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::ResetPseudoVelocities()
{
    for (ContactManifold& m : m_narrowPhase->GetManifoldCache().manifolds)
    {
        auto ResetPseudo = [&](EntityId e)
            {
                if (world->HasComponent<MotionComponent>(e))
                {
                    MotionComponent& mot = world->GetComponent<MotionComponent>(e);
                    mot.pseudoLinearVelocity = { 0,0,0 };
                    mot.pseudoAngularVelocity = { 0,0,0 };
                }
            };
        ResetPseudo(m.a);
        ResetPseudo(m.b);
    }
}

void PhysicSystem::UpdateWorldInertias()
{
    auto& manifolds = m_narrowPhase->GetManifoldCache().manifolds;

    for (ContactManifold& m : manifolds)
    {
        if (world->HasComponent<RigidBodyComponent>(m.a) &&
            world->HasComponent<TransformComponent>(m.a))
        {
            RigidBodyComponent& r = world->GetComponent<RigidBodyComponent>(m.a);
            if (r.type == BodyType::Dynamic && r.allowRotation)
                UpdateWorldInertia(r, world->GetComponent<TransformComponent>(m.a));
        }
        if (world->HasComponent<RigidBodyComponent>(m.b) &&
            world->HasComponent<TransformComponent>(m.b))
        {
            RigidBodyComponent& r = world->GetComponent<RigidBodyComponent>(m.b);
            if (r.type == BodyType::Dynamic && r.allowRotation)
                UpdateWorldInertia(r, world->GetComponent<TransformComponent>(m.b));
        }
    }
}

void PhysicSystem::UpdateWorldInertia(RigidBodyComponent& _rigid, TransformComponent& _transform) const
{
    // Charger I_body_inv en XMMATRIX.
    const float* bi = _rigid.inertiaTensorBodyInverse;
    XMMATRIX I_body_inv(
        bi[0],bi[1],bi[2],0,
        bi[3],bi[4],bi[5],0,
        bi[6],bi[7],bi[8],0,
        0,   0,   0,  1
    );

    XMVECTOR q  = XMLoadFloat4(&_transform.world.GetRotation());
    XMMATRIX R  = XMMatrixRotationQuaternion(q);
    XMMATRIX RT = XMMatrixTranspose(R);

    XMMATRIX I_world_inv = R * I_body_inv * RT;

    // Stocker en float[9].
    XMFLOAT4X4 tmp;
    XMStoreFloat4x4(&tmp, I_world_inv);
    float* wi = _rigid.inertiaTensorWorldInverse;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            wi[r*3+c] = tmp.m[r][c];
}

// ─────────────────────────────────────────────────────────────────────────────
// Warm start
//
//  Réinjecte kWarmStartFactor × les impulsions accumulées de la frame précédente.
//  Le solver part d'un état déjà proche de la solution → converge en moins
//  d'itérations avec moins de jitter.
//
//  On ne warm-start pas si les corps s'éloignent déjà (velocityAlongNormal ≥ 0).
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::WarmStart()
{
    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (!manifold.isWarm) continue;
        if (manifold.isTrigger) continue;

        if (!world->HasComponent<RigidBodyComponent>(manifold.a) ||
            !world->HasComponent<RigidBodyComponent>(manifold.b)) continue;

        RigidBodyComponent& rigidA = world->GetComponent<RigidBodyComponent>(manifold.a);
        RigidBodyComponent& rigidB = world->GetComponent<RigidBodyComponent>(manifold.b);

        if (!world->HasComponent<MotionComponent>(manifold.a) ||
            !world->HasComponent<MotionComponent>(manifold.b)) continue;

        MotionComponent& motionA = world->GetComponent<MotionComponent>(manifold.a);
        MotionComponent& motionB = world->GetComponent<MotionComponent>(manifold.b);

        if (motionA.isSleeping && motionB.isSleeping) continue;

        for (int i = 0; i < manifold.pointCount; ++i)
        {
            ContactPoint& cp = manifold.points[i];

            ContactCtx ctx = BuildCtx(motionA, motionB, manifold.a, manifold.b, cp.position);

            // Ne pas warm-starter un contact mourant.
            if (Dot(ctx.relativeVelocity, manifold.normal) >= 0.0f)
            {
                cp.accumulatedNormalImpulse   = 0.0f;
                cp.accumulatedTangentImpulse  = 0.0f;
                cp.accumulatedBinormalImpulse = 0.0f;
                continue;
            }

            // Impulsion normale.
            float warmN = cp.accumulatedNormalImpulse * kWarmStartFactor;
            XMFLOAT3 Jn = Mul(manifold.normal, warmN);
            ApplyImpulse(motionA, rigidA, Jn, ctx.rA, -1.0f);
            ApplyImpulse(motionB, rigidB, Jn, ctx.rB, +1.0f);

            // Recalculer le contexte pour la tangente.
            ctx = BuildCtx(motionA, motionB, manifold.a, manifold.b, cp.position);

            // Tangente 1.
            XMFLOAT3 tangent = Sub(ctx.relativeVelocity,
                                   Mul(manifold.normal, Dot(ctx.relativeVelocity, manifold.normal)));
            if (LenSq(tangent) > kTangentEpsilonSq)
            {
                tangent = Normalize(tangent);
                float warmT = cp.accumulatedTangentImpulse * kWarmStartFactor;
                XMFLOAT3 Jt = Mul(tangent, warmT);
                ApplyImpulse(motionA, rigidA, Jt, ctx.rA, -1.0f);
                ApplyImpulse(motionB, rigidB, Jt, ctx.rB, +1.0f);
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Correction positionnelle
//
//  Déplace les transforms pour éliminer la pénétration résiduelle.
//  Le slop (petite pénétration tolérée) évite le jitter des corps au repos.
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::ResolveOverlaps()
{
    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (manifold.isTrigger) continue;

        if (!world->HasComponent<RigidBodyComponent>(manifold.a) ||
            !world->HasComponent<RigidBodyComponent>(manifold.b)) continue;

        RigidBodyComponent& rigidA = world->GetComponent<RigidBodyComponent>(manifold.a);
        RigidBodyComponent& rigidB = world->GetComponent<RigidBodyComponent>(manifold.b);

        if (!world->HasComponent<MotionComponent>(manifold.a) ||
            !world->HasComponent<MotionComponent>(manifold.b)) continue;

        MotionComponent& motionA = world->GetComponent<MotionComponent>(manifold.a);
        MotionComponent& motionB = world->GetComponent<MotionComponent>(manifold.b);

        ResolveOverlap(rigidA, rigidB, motionA, motionB, manifold);
    }
}

void PhysicSystem::ResolveOverlap(RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
                                   MotionComponent& _motionA,   MotionComponent& _motionB,
                                   ContactManifold& _manifold)
{
    if (_manifold.penetration <= kPenetrationSlop)
        return;

    float depth = (_manifold.penetration - kPenetrationSlop) * kPenetrationPercent;

    float invMassA     = _rigidA.massInverse;
    float invMassB     = _rigidB.massInverse;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return;

    float moveA = depth * (invMassA / totalInvMass);
    float moveB = depth * (invMassB / totalInvMass);

    TransformComponent& tA = world->GetComponent<TransformComponent>(_manifold.a);
    TransformComponent& tB = world->GetComponent<TransformComponent>(_manifold.b);

    // Déplacer dans la direction opposée à la normale (A s'éloigne de B).
    if (moveA > 0.0f)
        tA.local.Move(Mul(_manifold.normal, -moveA));
    if (moveB > 0.0f)
        tB.local.Move(Mul(_manifold.normal, moveB));
}

// ─────────────────────────────────────────────────────────────────────────────
// Itérations PGS
//
//  Pour chaque itération :
//   Pour chaque manifold :
//    Pour chaque point de contact :
//     1. Résoudre la contrainte normale   (λn ≥ 0).
//     2. Résoudre la contrainte tangente1 (cône de Coulomb).
//     3. Résoudre la contrainte tangente2 (cône de Coulomb).
//
//  L'ordre normal → tangente est obligatoire : le clamping de la friction
//  dépend de l'accumulateur normal (Fn dans [-μFn, +μFn]).
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::SolveConstraints(int _iterations)
{
    for (int iter = 0; iter < _iterations; ++iter)
    {
        for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
        {
            if (manifold.isTrigger) continue;

            if (!world->HasComponent<RigidBodyComponent>(manifold.a) ||
                !world->HasComponent<RigidBodyComponent>(manifold.b)) continue;
            if (!world->HasComponent<MotionComponent>(manifold.a) ||
                !world->HasComponent<MotionComponent>(manifold.b)) continue;

            RigidBodyComponent& rigidA = world->GetComponent<RigidBodyComponent>(manifold.a);
            RigidBodyComponent& rigidB = world->GetComponent<RigidBodyComponent>(manifold.b);
            MotionComponent&    motionA = world->GetComponent<MotionComponent>(manifold.a);
            MotionComponent&    motionB = world->GetComponent<MotionComponent>(manifold.b);

            if (motionA.isSleeping && motionB.isSleeping) continue;

            for (int i = 0; i < manifold.pointCount; ++i)
            {
                ContactPoint& cp = manifold.points[i];

                // ── Normale ──────────────────────────────────────────────────
                ContactCtx ctx = BuildCtx(motionA, motionB, manifold.a, manifold.b, cp.position);
                SolveNormalConstraint(rigidA, rigidB, motionA, motionB, manifold, cp, ctx);

                // ── Tangentes ────────────────────────────────────────────────
                ctx = BuildCtx(motionA, motionB, manifold.a, manifold.b, cp.position);

                float maxFriction = 0.5f * (rigidA.staticFriction + rigidB.staticFriction)
                                  * cp.accumulatedNormalImpulse;

                XMFLOAT3 relVelInPlane = Sub(ctx.relativeVelocity,
                    Mul(manifold.normal, Dot(ctx.relativeVelocity, manifold.normal)));
                // Tangente 1 : dans le plan de contact, depuis la vitesse relative.
                XMFLOAT3 t1;
                if (LenSq(relVelInPlane) > kTangentEpsilonSq)
                {
                    t1 = Normalize(relVelInPlane);
                }
                else
                {
                    // Tangente arbitraire perpendiculaire à la normale.
                    // Stable même à vitesse nulle.
                    XMFLOAT3 up = { 0, 1, 0 };
                    t1 = Cross(manifold.normal, up);
                    if (LenSq(t1) < kTangentEpsilonSq)
                        t1 = Cross(manifold.normal, { 1, 0, 0 });
                    t1 = Normalize(t1);
                }

                // Tangente 2 : perpendiculaire à normale et t1.
                ctx = BuildCtx(motionA, motionB, manifold.a, manifold.b, cp.position);
                XMFLOAT3 t2 = Cross(manifold.normal, t1);
                if (LenSq(t2) > kTangentEpsilonSq)
                {
                    t2 = Normalize(t2);
                    SolveTangentConstraint(rigidA, rigidB, motionA, motionB,
                                           maxFriction, manifold, cp, ctx, t2,
                                           cp.accumulatedBinormalImpulse);
                }
            }
        }
    }
}

void PhysicSystem::SolvePositionConstraints(int _iterations)
{
    constexpr float kBeta = 0.5f;
    constexpr float kSlop = 0.005f;

    for (int iter = 0; iter < _iterations; ++iter)
    {
        for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
        {
            if (manifold.isTrigger) continue;

            if (!world->HasComponent<RigidBodyComponent>(manifold.a) ||
                !world->HasComponent<RigidBodyComponent>(manifold.b)) continue;
            if (!world->HasComponent<MotionComponent>(manifold.a) ||
                !world->HasComponent<MotionComponent>(manifold.b)) continue;

            RigidBodyComponent& rigidA = world->GetComponent<RigidBodyComponent>(manifold.a);
            RigidBodyComponent& rigidB = world->GetComponent<RigidBodyComponent>(manifold.b);
            MotionComponent& motionA = world->GetComponent<MotionComponent>(manifold.a);
            MotionComponent& motionB = world->GetComponent<MotionComponent>(manifold.b);

            float correction = Max(0.0f, manifold.penetration - kSlop) * kBeta;
            if (correction <= 0.0f) continue;

            float invMassA = rigidA.massInverse;
            float invMassB = rigidB.massInverse;
            float totalInvMass = invMassA + invMassB;
            if (totalInvMass <= 0.0f) continue;

            // Pseudo-vitesse séparatrice — ne touche pas linearVelocity.
            float pseudoA = correction * (invMassA / totalInvMass);
            float pseudoB = correction * (invMassB / totalInvMass);

            if (rigidA.type == BodyType::Dynamic)
            {
                motionA.pseudoLinearVelocity.x -= manifold.normal.x * pseudoA;
                motionA.pseudoLinearVelocity.y -= manifold.normal.y * pseudoA;
                motionA.pseudoLinearVelocity.z -= manifold.normal.z * pseudoA;
            }
            if (rigidB.type == BodyType::Dynamic)
            {
                motionB.pseudoLinearVelocity.x += manifold.normal.x * pseudoB;
                motionB.pseudoLinearVelocity.y += manifold.normal.y * pseudoB;
                motionB.pseudoLinearVelocity.z += manifold.normal.z * pseudoB;
            }
        }
    }
}
// ─────────────────────────────────────────────────────────────────────────────
// Contrainte normale — accumulation clampée à [0, +∞]
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::SolveNormalConstraint(
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    MotionComponent& _motionA, MotionComponent& _motionB,
    ContactManifold& _manifold, ContactPoint& _cp,
    const ContactCtx& _ctx)
{
    if (_rigidA.massInverse + _rigidB.massInverse <= 0.0f)
        return;

    float vn = Dot(_ctx.relativeVelocity, _manifold.normal);

    float restitution = Min(_rigidA.restitution, _rigidB.restitution);
    if (Abs(vn) < kRestitutionThreshold)
        restitution = 0.0f;

    float effectiveMass =
        _rigidA.massInverse + _rigidB.massInverse +
        ComputeAngularMassTerm(_ctx.rA, _manifold.normal, _rigidA.inertiaTensorWorldInverse) +
        ComputeAngularMassTerm(_ctx.rB, _manifold.normal, _rigidB.inertiaTensorWorldInverse);

    if (effectiveMass <= 0.0f) return;

    float lambda = -(1.0f + restitution) * vn / (effectiveMass * _manifold.pointCount);

    float oldAccum = _cp.accumulatedNormalImpulse;
    _cp.accumulatedNormalImpulse = Max(0.0f, oldAccum + lambda);
    float delta = _cp.accumulatedNormalImpulse - oldAccum;

    if (Abs(delta) < 1e-8f) return;

    XMFLOAT3 J = Mul(_manifold.normal, delta);
    ApplyImpulse(_motionA, _rigidA, J, _ctx.rA, -1.0f);
    ApplyImpulse(_motionB, _rigidB, J, _ctx.rB, +1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Contrainte tangentielle — cône de Coulomb [-μFn, +μFn]
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::SolveTangentConstraint(
    RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
    MotionComponent&    _motionA, MotionComponent&   _motionB,
    float _maxFriction,
    ContactManifold&    _manifold, ContactPoint&     _cp,
    const ContactCtx&   _ctx,
    const XMFLOAT3& _tangent, float& _accumulated)
{
    float vt = Dot(_ctx.relativeVelocity, _tangent);

    float effectiveMass =
        _rigidA.massInverse + _rigidB.massInverse +
        ComputeAngularMassTerm(_ctx.rA, _tangent, _rigidA.inertiaTensorWorldInverse) +
        ComputeAngularMassTerm(_ctx.rB, _tangent, _rigidB.inertiaTensorWorldInverse);

    if (effectiveMass <= 0.0f) return;

    float lambda = -vt / (effectiveMass * _manifold.pointCount);

    // Cône de Coulomb.
    float staticFriction  = 0.5f * (_rigidA.staticFriction  + _rigidB.staticFriction);
    float dynamicFriction = 0.5f * (_rigidA.dynamicFriction + _rigidB.dynamicFriction);
    float maxStatic  = staticFriction  * _cp.accumulatedNormalImpulse;
    float maxDynamic = dynamicFriction * _cp.accumulatedNormalImpulse;

    float oldAccum  = _accumulated;
    float newAccum  = oldAccum + lambda;

    if (Abs(newAccum) <= maxStatic)
        _accumulated = newAccum;                              // friction statique
    else
        _accumulated = (newAccum > 0.0f ? 1.0f : -1.0f) * maxDynamic; // friction dynamique

    float delta = _accumulated - oldAccum;
    if (Abs(delta) < 1e-8f) return;

    std::cout << "[FRICTION]"
        << " vt=" << vt
        << " lambda=" << lambda
        << " oldAccum=" << oldAccum
        << " newAccum=" << _accumulated
        << " maxStatic=" << maxStatic
        << " maxDynamic=" << maxDynamic
        << " delta=" << delta
        << "\n";

    XMFLOAT3 J = Mul(_tangent, delta);
    ApplyImpulse(_motionA, _rigidA, J, _ctx.rA, -1.0f);
    ApplyImpulse(_motionB, _rigidB, J, _ctx.rB, +1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Réveil des corps
//
//  Un corps endormi posé sur un corps dynamique en mouvement doit se réveiller.
//  On ne réveille que sur un impact significatif pour éviter de réveiller
//  des corps en repos lors de micro-contacts.
// ─────────────────────────────────────────────────────────────────────────────

void PhysicSystem::WakeBodies()
{
    for (ContactManifold& manifold : m_narrowPhase->GetManifoldCache().manifolds)
    {
        if (!world->HasComponent<MotionComponent>(manifold.a) ||
            !world->HasComponent<MotionComponent>(manifold.b)) continue;

        MotionComponent& motionA = world->GetComponent<MotionComponent>(manifold.a);
        MotionComponent& motionB = world->GetComponent<MotionComponent>(manifold.b);

        XMFLOAT3 relVel = Sub(motionB.linearVelocity, motionA.linearVelocity);
        float vn = Dot(relVel, manifold.normal);

        if (vn < -kWakeThreshold)
        {
            motionA.WakeUp();
            motionB.WakeUp();
        }

        if (!motionA.isSleeping && motionB.isSleeping) motionB.WakeUp();
        if (!motionB.isSleeping && motionA.isSleeping) motionA.WakeUp();

        // Stabilisation angulaire au repos.
        // Le solver génère du torque parasite via r × J sur les coins.
        // On supprime la composante latérale (hors spin autour de la normale)
        // quand le contact est quasi-statique.
        float linearSpeedA = LenSq(motionA.linearVelocity);
        float linearSpeedB = LenSq(motionB.linearVelocity);
        constexpr float kRestingThreshold = 0.25f; // 0.5 m/s au carré

        bool isResting = Abs(vn) < 0.5f
            && linearSpeedA < kRestingThreshold
            && linearSpeedB < kRestingThreshold
            && manifold.penetration < 0.02f;

        if (!isResting) continue;

        auto StabilizeAngular = [&](MotionComponent& motion)
            {
                if (motion.isSleeping) return;

                // Conserver uniquement le spin autour de la normale.
                float spinN = Dot(motion.angularVelocity, manifold.normal);
                XMFLOAT3 spin = Mul(manifold.normal, spinN);
                XMFLOAT3 lateral = Sub(motion.angularVelocity, spin);

                constexpr float kAngularRestThreshold = 0.1f;
                if (LenSq(lateral) < kAngularRestThreshold * kAngularRestThreshold)
                    motion.angularVelocity = spin;
                else
                    motion.angularVelocity = Add(spin, Mul(lateral, 0.7f));
            };

        StabilizeAngular(motionA);
        StabilizeAngular(motionB);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

PhysicSystem::ContactCtx PhysicSystem::BuildCtx(
    MotionComponent& _motionA, MotionComponent& _motionB,
    EntityId _eA, EntityId _eB, const XMFLOAT3& _point) const
{
    ContactCtx ctx;
    ctx.rA = Sub(_point, GetCenter(_eA));
    ctx.rB = Sub(_point, GetCenter(_eB));

    // v_point = v_linear + ω × r
    XMFLOAT3 vA = _motionA.GetVelocityAtPoint(ctx.rA);
    XMFLOAT3 vB = _motionB.GetVelocityAtPoint(ctx.rB);

    ctx.relativeVelocity = Sub(vB, vA);
    return ctx;
}

void PhysicSystem::ApplyImpulse(MotionComponent& _motion, RigidBodyComponent& _rigid,
                                  const XMFLOAT3& _impulse, const XMFLOAT3& _r, float _sign) const
{
    if (_rigid.type == BodyType::Static || _rigid.type == BodyType::Kinematic)
        return;

    // Δv = ±J / m
    _motion.linearVelocity.x += _impulse.x * _rigid.massInverse * _sign;
    _motion.linearVelocity.y += _impulse.y * _rigid.massInverse * _sign;
    _motion.linearVelocity.z += _impulse.z * _rigid.massInverse * _sign;

    if (_rigid.allowRotation)
    {
        // Δω = ±I_world_inv · (r × J)
        XMFLOAT3 angImpulse = Cross(_r, _impulse);
        XMFLOAT3 deltaOmega = ApplyInertiaInverse(angImpulse, _rigid.inertiaTensorWorldInverse);

        _motion.angularVelocity.x += deltaOmega.x * _sign;
        _motion.angularVelocity.y += deltaOmega.y * _sign;
        _motion.angularVelocity.z += deltaOmega.z * _sign;
    }
}

XMFLOAT3 PhysicSystem::ApplyInertiaInverse(const XMFLOAT3& _v, const float _tensor[9]) const
{
    // Multiplication matrice 3×3 × vecteur 3.
    return
    {
        _tensor[0]*_v.x + _tensor[1]*_v.y + _tensor[2]*_v.z,
        _tensor[3]*_v.x + _tensor[4]*_v.y + _tensor[5]*_v.z,
        _tensor[6]*_v.x + _tensor[7]*_v.y + _tensor[8]*_v.z
    };
}

float PhysicSystem::ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis,
                                            const float _tensorInv[9]) const
{
    // (r × axis)^T · I_inv · (r × axis)
    XMFLOAT3 rxn    = Cross(_r, _axis);
    XMFLOAT3 i_rxn  = ApplyInertiaInverse(rxn, _tensorInv);
    XMFLOAT3 result = Cross(i_rxn, _r);
    return Dot(result, _axis);
}

XMFLOAT3 PhysicSystem::GetCenter(EntityId _e) const
{
    TransformComponent& t = world->GetComponent<TransformComponent>(_e);
    return t.local.GetPosition();
}
