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
        XMFLOAT3 rA = { 0,0,0 };
        XMFLOAT3 rB = { 0,0,0 };

        bool active = false;

        float accumulatedNormalImpulse = 0.0f;
        float lastAppliedNormalImpulse = 0.0f;
    };

    struct CachedCollision
    {
        const CollisionResult* collision = nullptr;
        ContactInfo contact;
        CachedContactPoint points[kMaxContactPoints];
    };

    void BuildFrameCollisionCache();
    void ResolveVelocities(float _dt);
    void ResolvePenetrations();

    void ResolveCollisionVelocities(CachedCollision& _cachedCollision);

    void SolveNormalImpulses(
        const ContactInfo& _contact,
        MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        CachedContactPoint* _cache);

    void SolveFrictionImpulses(
        const ContactInfo& _contact,
        MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const CachedContactPoint* _cache);

    void BuildContactPointCache(
        const ContactInfo& _contact,
        const XMFLOAT3& _centerA, const XMFLOAT3& _centerB,
        CachedContactPoint* _cache);

    void ApplyFriction(MotionComponent& _motionA, MotionComponent& _motionB,
        RigidBodyComponent& _rigidA, RigidBodyComponent& _rigidB,
        const XMFLOAT3& _rA, const XMFLOAT3& _rB,
        const XMFLOAT3& _normal, const XMFLOAT3& _relativeVelocityPreSolve,
        float _normalImpulse, int _pointCount);

    void ApplyAngularPositionCorrection(EntityId _entity,
        RigidBodyComponent& _rigid,
        TransformComponent& _transform,
        const XMFLOAT3& _contactPoint,
        const XMFLOAT3& _normal,
        float _angularAmount) const;

    void WakeSleepingPair(MotionComponent& _motionA, MotionComponent& _motionB,
        const RigidBodyComponent& _rigidA, const RigidBodyComponent& _rigidB);

    MotionComponent& GetMotion(EntityId _entity);
    RigidBodyComponent* GetRigidBody(EntityId _entity);
    XMFLOAT3 GetBodyCenter(EntityId _entity) const;

    float GetMaxPenetration(const ContactInfo& _contact) const;
    ContactInfo BuildReducedContact(const ContactInfo& _contact) const;
    bool ShouldReduceToCenterPoint(const ContactInfo& _contact) const;

    XMFLOAT3 VelocityAtPoint(const MotionComponent& _motion, const XMFLOAT3& _r) const;
    XMFLOAT3 ApplyInertiaInverse(const XMFLOAT3& _v, const float _tensor[9]) const;
    XMFLOAT3 ComputeAverageContactPoint(const ContactInfo& _contact) const;
    float ComputeAngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _tensor[9]) const;
    void RotateTransformByAxisAngle(TransformComponent& _transform, const XMFLOAT3& _axisAngle) const;

    void NotifyScripts();

private:
    NarrowPhaseSystem* m_narrowPhase = nullptr;
    MotionComponent m_nullMotion;

    Vector<CachedCollision> m_collisionCache;

    static constexpr int   kVelocityIterations = 8;
    static constexpr int   kPositionIterations = 3;

    static constexpr float kLinearSnapThreshold = 0.1f;
    static constexpr float kAngularSnapThreshold = 0.1f;

    static constexpr float kRestitutionThreshold = 0.3f;
    static constexpr float kPenetrationSlop = 0.01f;
    static constexpr float kBaumgarteBeta = 0.1f;
    static constexpr float kAngularPositionBeta = 0.03f;
    static constexpr float kMaxAngularPositionCorrection = 0.05f;
};

#endif
