#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/MotionComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "NarrowPhaseSystem.h"

// ─────────────────────────────────────────────────────────────────────────────
// PhysicSystem — solver PGS (Projected Gauss-Seidel)
//
//  Pipeline par frame (dans OnEndUpdate) :
//   1. UpdateWorldInertias — met à jour I_world_inv pour chaque corps Dynamic.
//   2. WarmStart           — réinjecte les impulsions de la frame précédente.
//   3. ResolveOverlaps     — correction positionnelle.
//   4. SolveConstraints    — N itérations PGS sur tous les manifolds.
//   5. WakeBodies          — réveille les corps en contact significatif.
//
//  Accumulation clampée (PGS) :
//   Au lieu d'appliquer l'impulsion calculée directement, on l'ajoute à un
//   accumulateur clampé. Le delta réellement appliqué est la différence entre
//   le nouvel accumulateur et l'ancien. Ce clamping est ce qui stabilise
//   les empilements — sans lui le solver oscille.
//
//  Warm start :
//   Les accumulateurs sont persistés entre frames par ContactManifoldCache.
//   Au début de chaque frame on les réinjecte (× kWarmStartFactor) pour que
//   le solver parte d'un état déjà proche de la solution.
//
//  Ordre d'exécution : après NarrowPhaseSystem dans Phase::FixedUpdate.
// ─────────────────────────────────────────────────────────────────────────────
class PhysicSystem : public System<RigidBodyComponent, MotionComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override {}
    void OnUpdate(float _dt, EntityId _e,
                  RigidBodyComponent& _rigid,
                  MotionComponent&    _motion,
                  TransformComponent& _transform) override {}
    void OnEndUpdate(float _dt) override;

    void SetNarrowPhaseSystem(NarrowPhaseSystem* _np) { m_narrowPhase = _np; }

private:
    // ─── Contexte par point de contact ───────────────────────────────────────
    struct ContactCtx
    {
        XMFLOAT3 rA;               // centerA → point de contact
        XMFLOAT3 rB;               // centerB → point de contact
        XMFLOAT3 relativeVelocity; // vitesse relative au point (vB - vA)
    };

    // ─── Pipeline ─────────────────────────────────────────────────────────────
    void ResetPseudoVelocities();

    void UpdateWorldInertias();
    void WarmStart();
    void ResolveOverlaps();
    void SolveConstraints(int _iterations);
	void SolvePositionConstraints(int _iterations);
    void WakeBodies();

    // ─── Résolution par contact ───────────────────────────────────────────────
    void ResolveOverlap(RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
                        MotionComponent& _motionA,   MotionComponent& _motionB,
                        ContactManifold& _manifold);

    void SolveNormalConstraint(RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
                               MotionComponent& _motionA,   MotionComponent& _motionB,
                               ContactManifold& _manifold,  ContactPoint& _cp,
                               const ContactCtx& _ctx);

    void SolveTangentConstraint(RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
                                MotionComponent& _motionA,   MotionComponent& _motionB,
                                float _maxFriction,
                                ContactManifold& _manifold,  ContactPoint& _cp,
                                const ContactCtx& _ctx,
                                const XMFLOAT3& _tangent,    float& _accumulated);

    // ─── Helpers ──────────────────────────────────────────────────────────────
    ContactCtx BuildCtx(MotionComponent& _motionA, MotionComponent& _motionB,
                        EntityId _eA, EntityId _eB, const XMFLOAT3& _point) const;

    void ApplyImpulse(MotionComponent& _motion, RigidBodyComponent& _rigid,
                      const XMFLOAT3& _impulse, const XMFLOAT3& _r, float _sign) const;

    XMFLOAT3 ApplyInertiaInverse(const XMFLOAT3& _v, const float _tensor[9]) const;
    float    ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _tensorInv[9]) const;
    void     UpdateWorldInertia(RigidBodyComponent& _rigid, TransformComponent& _transform) const;
    XMFLOAT3 GetCenter(EntityId _e) const;

    inline XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b) const
    { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
    inline float    Dot(const XMFLOAT3& a, const XMFLOAT3& b) const
    { return a.x*b.x+a.y*b.y+a.z*b.z; }
    inline XMFLOAT3 Sub(const XMFLOAT3& a, const XMFLOAT3& b) const
    { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
    inline XMFLOAT3 Add(const XMFLOAT3& a, const XMFLOAT3& b) const
    { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
    inline XMFLOAT3 Mul(const XMFLOAT3& v, float s) const
    { return {v.x*s,v.y*s,v.z*s}; }
    inline float LenSq(const XMFLOAT3& v) const
    { return v.x*v.x+v.y*v.y+v.z*v.z; }
    inline XMFLOAT3 Normalize(const XMFLOAT3& v) const
    { float l=sqrtf(LenSq(v)); return l<1e-8f?XMFLOAT3{0,1,0}:Mul(v,1.f/l); }
    inline float Max(float a, float b) const { return a>b?a:b; }
    inline float Min(float a, float b) const { return a<b?a:b; }
    inline float Clamp(float v, float lo, float hi) const { return Max(lo,Min(hi,v)); }
    inline float Abs(float v) const { return v<0?-v:v; }

private:
    NarrowPhaseSystem* m_narrowPhase = nullptr;

    static constexpr int   kSolverIterations    = 10;
    static constexpr float kWarmStartFactor      = 0.8f;
    static constexpr float kPenetrationSlop      = 0.005f;
    static constexpr float kPenetrationPercent   = 0.8f;
    static constexpr float kRestitutionThreshold = 0.2f;
    static constexpr float kTangentEpsilonSq     = 1e-6f;
    static constexpr float kWakeThreshold        = 0.3f;
};

#endif // !PHYSIC_SYSTEM_H_DEFINED
