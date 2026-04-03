#ifndef PHYSIC_INTEGRATE_SYSTEM_H_DEFINED
#define PHYSIC_INTEGRATE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/MotionComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"

class PhysicIntegrateSystem : public System<RigidBodyComponent, MotionComponent, ColliderComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override {}

    void OnUpdate(float _dt, EntityId _e,
                  RigidBodyComponent& _rigid,
                  MotionComponent&    _motion,
                  ColliderComponent&     _shape,
                  TransformComponent& _transform) override;

    void OnEndUpdate(float _dt) override {}

    void SetGravity(const XMFLOAT3& _g) { m_gravity = _g; }

private:
    void ComputeBodyInertiaTensor (RigidBodyComponent& _rigid, ColliderComponent& _shape);
    void UpdateWorldInertiaTensor (RigidBodyComponent& _rigid, TransformComponent& _transform);

    XMFLOAT3 IntegrateLinearVelocity  (RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt);
    XMFLOAT3 IntegrateAngularVelocity (RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt);
    void     UpdateQuaternion         (TransformComponent& _transform, const XMFLOAT3& _deltaAngle);

    XMFLOAT3 m_gravity = { 0.0f, -9.81f, 0.0f };

    static constexpr float kSleepLinearThreshold  = 0.1f;
    static constexpr float kSleepAngularThreshold = 0.1f;
    static constexpr float kSleepTimeThreshold    = 0.5f;
    static constexpr float kMinAngularVelocitySq  = 0.05f;
};

#endif // !PHYSIC_INTEGRATE_SYSTEM_H_DEFINED
