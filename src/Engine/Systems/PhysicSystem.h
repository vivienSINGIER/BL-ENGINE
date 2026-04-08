#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/MotionComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "NarrowPhaseSystem.h"
#include "../Core/Utils.hpp"

class PhysicSystem : public System<>
{
public:
    void Update(float _dt) override;
    void SetNarrowPhaseSystem(NarrowPhaseSystem* _narrowPhase) { m_narrowPhase = _narrowPhase; }

private:
    struct CachedContactPoint
    {
        XMFLOAT3 rA = { 0, 0, 0 };
        XMFLOAT3 rB = { 0, 0, 0 };
        XMFLOAT3 relativeVelocity = { 0, 0, 0 };
        float normalImpulse = 0.0f;
        float biasImpulse = 0.0f;
        bool active = false;
    };

    void ResolveVelocities(float _dt);
    void ResolvePenetrations();

    void ResolveCollisionVelocities(const CollisionResult& _collision, float _invDt);
    void SolveNormalImpulses(const ContactInfo& _contact,
        MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const CachedContactPoint* _cache);
    void SolveFrictionImpulses(const ContactInfo& _contact,
        MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const CachedContactPoint* _cache);

    void BuildContactPointCache(const ContactInfo& _contact,
        MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const XMFLOAT3& _centerA, const XMFLOAT3& _centerB,
        float _invDt,
        CachedContactPoint* _cache);

    void ApplyFriction(MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const XMFLOAT3& _rA, const XMFLOAT3& _rB,
        const XMFLOAT3& _normal, const XMFLOAT3& _relativeVelocityPreSolve,
        float _normalImpulse, int _pointCount);

    void WakeSleepingPair(MotionComponent& _motionA, MotionComponent& _motionB,
        const RigidBodyComponent& _rigidA, const RigidBodyComponent& _rigidB);

    MotionComponent& GetMotion(EntityId _entity);
    RigidBodyComponent* GetRigidBody(EntityId _entity);
    XMFLOAT3 GetBodyCenter(EntityId _entity) const;
    float GetMaxPenetration(const ContactInfo& _contact) const;

    XMFLOAT3 VelocityAtPoint(const MotionComponent& _motion, const XMFLOAT3& _r) const;
    XMFLOAT3 ApplyInertiaInverse(const XMFLOAT3& _v, const float _tensor[9]) const;
    float ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _tensor[9]) const;

private:
    NarrowPhaseSystem* m_narrowPhase = nullptr;
    MotionComponent m_nullMotion;

    static constexpr int   kVelocityIterations = 8;
    static constexpr int   kPositionIterations = 3;

    static constexpr float kLinearSnapThreshold = 0.1f;
    static constexpr float kAngularSnapThreshold = 0.1f;

    static constexpr float kRestitutionThreshold = 0.3f;
    static constexpr float kPenetrationSlop = 0.01f;
    static constexpr float kBaumgarteBeta = 0.3f;
};

#endif
