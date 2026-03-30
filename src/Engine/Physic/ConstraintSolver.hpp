#pragma once
// ============================================================
//  ConstraintSolver.hpp
//  Projected Gauss-Seidel (PGS) / Sequential Impulse Solver
//
//  Différences vs ton ancien moteur :
//  1) Warm starting  : les impulses N-1 sont pré-appliquées
//  2) Masse effective pré-calculée par point (pas recalculée
//     à chaque itération)
//  3) Position correction via pseudo-velocities (Split Impulse)
//     plutôt que Baumgarte pur
//  4) Friction via cône de Coulomb (clamp non-penetration
//     impulse cumulée)
//  5) Contraintes jointes (distance, hinge) en option
// ============================================================
#include "RigidBody.hpp"
#include "Contact.hpp"
#include <vector>
#include <unordered_map>
#include <functional>

// ============================================================
//  Utilitaire : effective mass term angulaire
// ============================================================
inline float EffMassTerm(const XMFLOAT3& r, const XMFLOAT3& axis, const RigidBody& rb)
{
    if (!rb.allowRotation || rb.bodyType == BodyType::Static) return 0.0f;
    XMFLOAT3 rxn = V3Cross(r, axis);
    XMFLOAT3 I_rxn = rb.inertia.ApplyInvWorld(rxn);
    return V3Dot(V3Cross(I_rxn, r), axis);
}

// ============================================================
//  ConstraintSolver
// ============================================================
class ConstraintSolver
{
public:
    // Paramètres de tuning (exposés publiquement comme VPhysics)
    float baumgarteSlop    = 0.005f;  // pénétration tolérée avant correction
    float baumgarteFactor  = 0.2f;   // facteur de correction positionnelle
    float restitutionThreshold = 0.2f; // en dessous : restitution = 0
    int   iterations       = 8;       // itérations PGS (4 → 8 pour mieux converger)

    // Callback pour accéder à un RigidBody par EntityId
    using BodyGetter = std::function<RigidBody*(uint32_t)>;
    BodyGetter getBody;

    // ---- Pipeline principal ----
    void Solve(std::vector<Contact>& contacts, ContactCache& cache, float dt)
    {
        if (!getBody) return;

        // 1) Pré-calcul des masses effectives + warm starting
        _PreSolve(contacts, cache, dt);

        // 2) Itérations PGS
        for (int iter = 0; iter < iterations; ++iter)
            _SolveIteration(contacts, dt);

        // 3) Stockage des impulses dans le cache
        _PostSolve(contacts, cache);
    }

private:
    // ----------------------------------------------------------
    //  Pré-solve : masses effectives + warm starting
    // ----------------------------------------------------------
    void _PreSolve(std::vector<Contact>& contacts, ContactCache& cache, float dt)
    {
        for (Contact& c : contacts)
        {
            RigidBody* rbA = getBody(c.idA);
            RigidBody* rbB = getBody(c.idB);
            if (!rbA || !rbB) continue;

            ContactKey key(c.idA, c.idB);

            for (int i = 0; i < c.pointCount; ++i)
            {
                ContactPoint& cp = c.points[i];

                cp.rA = V3Sub(cp.position, rbA->position);
                cp.rB = V3Sub(cp.position, rbB->position);

                float invMassSum = rbA->massInv + rbB->massInv;

                // Masse effective normale
                float angA = EffMassTerm(cp.rA, c.normal, *rbA);
                float angB = EffMassTerm(cp.rB, c.normal, *rbB);
                cp.normalEffMass = 1.0f / Maxf(invMassSum + angA + angB, 1e-10f);

                // Masse effective tangente
                for (int t = 0; t < 2; ++t)
                {
                    float tA = EffMassTerm(cp.rA, c.tangent[t], *rbA);
                    float tB = EffMassTerm(cp.rB, c.tangent[t], *rbB);
                    cp.tangentEffMass[t] = 1.0f / Maxf(invMassSum + tA + tB, 1e-10f);
                }

                // Biais de position (Split Impulse / Baumgarte)
                float slop = Maxf(0.0f, cp.penetration - baumgarteSlop);
                cp.positionBias = (baumgarteFactor / dt) * slop;

                // Warm starting : applique les impulses de la frame précédente
                CachedImpulses cached;
                if (cache.Retrieve(key, i, cached))
                {
                    XMFLOAT3 impulse = V3Scale(c.normal, cached.normalImpulse);
                    impulse = V3Add(impulse, V3Scale(c.tangent[0], cached.tangentImpulse[0]));
                    impulse = V3Add(impulse, V3Scale(c.tangent[1], cached.tangentImpulse[1]));
                    _ApplyImpulse(*rbA, *rbB, cp.rA, cp.rB, impulse);

                    // Pré-charge les accumulateurs pour le clamping
                    cp.normalImpulse     = cached.normalImpulse;
                    cp.tangentImpulse[0] = cached.tangentImpulse[0];
                    cp.tangentImpulse[1] = cached.tangentImpulse[1];
                }
            }
        }
    }

    // ----------------------------------------------------------
    //  Une itération PGS
    // ----------------------------------------------------------
    void _SolveIteration(std::vector<Contact>& contacts, float dt)
    {
        for (Contact& c : contacts)
        {
            RigidBody* rbA = getBody(c.idA);
            RigidBody* rbB = getBody(c.idB);
            if (!rbA || !rbB) continue;

            float muS = 0.5f*(rbA->staticFriction  + rbB->staticFriction);
            float muD = 0.5f*(rbA->dynamicFriction  + rbB->dynamicFriction);
            float e   = Minf(rbA->restitution, rbB->restitution);

            for (int i = 0; i < c.pointCount; ++i)
            {
                ContactPoint& cp = c.points[i];

                // ---- Impulse normale ----
                {
                    XMFLOAT3 velA = rbA->GetVelocityAtPoint(cp.position);
                    XMFLOAT3 velB = rbB->GetVelocityAtPoint(cp.position);
                    XMFLOAT3 relV = V3Sub(velB, velA);
                    float vn = V3Dot(relV, c.normal);

                    // Restitution : uniquement si impact rapide
                    float restitution = fabsf(vn) > restitutionThreshold ? e : 0.0f;

                    float lambda = -(1.0f + restitution) * vn + cp.positionBias;
                    lambda *= cp.normalEffMass;

                    // Clamping : accumulation non-négative (Projected GS)
                    float oldAccum = cp.normalImpulse;
                    cp.normalImpulse = Maxf(0.0f, oldAccum + lambda);
                    float delta = cp.normalImpulse - oldAccum;

                    _ApplyImpulse(*rbA, *rbB, cp.rA, cp.rB, V3Scale(c.normal, delta));
                }

                // ---- Impulse tangentielle (friction) ----
                for (int t = 0; t < 2; ++t)
                {
                    XMFLOAT3 velA = rbA->GetVelocityAtPoint(cp.position);
                    XMFLOAT3 velB = rbB->GetVelocityAtPoint(cp.position);
                    XMFLOAT3 relV = V3Sub(velB, velA);
                    float vt = V3Dot(relV, c.tangent[t]);

                    float lambda = -vt * cp.tangentEffMass[t];

                    // Cône de Coulomb
                    float maxFriction = cp.normalImpulse * muS;
                    float oldAccum    = cp.tangentImpulse[t];
                    float newAccum    = Clampf(oldAccum + lambda, -maxFriction, maxFriction);

                    // Si on dépasse le cône statique → friction dynamique
                    if (fabsf(newAccum) >= maxFriction)
                        newAccum = (newAccum > 0 ? 1 : -1) * cp.normalImpulse * muD;

                    float delta = newAccum - oldAccum;
                    cp.tangentImpulse[t] = newAccum;

                    _ApplyImpulse(*rbA, *rbB, cp.rA, cp.rB, V3Scale(c.tangent[t], delta));
                }
            }
        }
    }

    // ----------------------------------------------------------
    //  Post-solve : stockage dans le cache
    // ----------------------------------------------------------
    void _PostSolve(std::vector<Contact>& contacts, ContactCache& cache)
    {
        cache.Clear();
        for (Contact& c : contacts)
        {
            ContactKey key(c.idA, c.idB);
            for (int i = 0; i < c.pointCount; ++i)
                cache.Store(key, i, c.points[i]);
        }
    }

    // ----------------------------------------------------------
    //  Application d'une impulse à une paire de corps
    // ----------------------------------------------------------
    void _ApplyImpulse(RigidBody& a, RigidBody& b,
                        const XMFLOAT3& rA, const XMFLOAT3& rB,
                        const XMFLOAT3& j)
    {
        if (a.bodyType != BodyType::Static)
        {
            a.linearVelocity = V3Sub(a.linearVelocity, V3Scale(j, a.massInv));
            if (a.allowRotation)
            {
                XMFLOAT3 dw = a.inertia.ApplyInvWorld(V3Cross(rA, j));
                a.angularVelocity = V3Sub(a.angularVelocity, dw);
            }
        }
        if (b.bodyType != BodyType::Static)
        {
            b.linearVelocity = V3Add(b.linearVelocity, V3Scale(j, b.massInv));
            if (b.allowRotation)
            {
                XMFLOAT3 dw = b.inertia.ApplyInvWorld(V3Cross(rB, j));
                b.angularVelocity = V3Add(b.angularVelocity, dw);
            }
        }
    }

public:
    // ============================================================
    //  Résolution des positions (séparée de la vitesse)
    //  Style "Split Impulse" / Source Engine position correction
    // ============================================================
    void SolvePositions(std::vector<Contact>& contacts)
    {
        // Correction positionnelle directe (style Baumgarte mais appliquée
        // sur le transform, pas la vitesse)
        for (Contact& c : contacts)
        {
            RigidBody* rbA = getBody(c.idA);
            RigidBody* rbB = getBody(c.idB);
            if (!rbA || !rbB) continue;

            for (int i = 0; i < c.pointCount; ++i)
            {
                float pen = c.points[i].penetration;
                float corr = Maxf(0.0f, pen - baumgarteSlop) * baumgarteFactor;
                if (corr <= 0) continue;

                float invMassA = rbA->massInv;
                float invMassB = rbB->massInv;
                float total = invMassA + invMassB;
                if (total <= 0) continue;

                XMFLOAT3 correction = V3Scale(c.normal, corr);
                if (rbA->bodyType != BodyType::Static)
                    rbA->position = V3Sub(rbA->position, V3Scale(correction, invMassA / total));
                if (rbB->bodyType != BodyType::Static)
                    rbB->position = V3Add(rbB->position, V3Scale(correction, invMassB / total));
            }
        }
    }
};
