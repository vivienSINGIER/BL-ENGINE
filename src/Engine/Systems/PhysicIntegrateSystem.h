#ifndef PHYSIC_INTEGRATE_SYSTEM_H_DEFINED
#define PHYSIC_INTEGRATE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/MotionComponent.hpp"
#include "../Components/ShapeComponent.hpp"
#include "../Components/TransformComponent.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// PhysicIntegrateSystem
//
//  Dernier système de la phase FixedUpdate. Responsabilités :
//
//   1. ComputeBodyInertiaTensor  — calcule I_body depuis la géométrie du shape.
//                                  Recalculé chaque frame.
//
//   2. UpdateWorldInertiaTensor  — I_world_inv = R * I_body_inv * R^T.
//
//   3. IntegrateLinearVelocity   — forces + gravité → vitesse (Euler symplectique).
//
//   4. IntegrateAngularVelocity  — torque + I_world_inv → vitesse angulaire.
//
//   5. Intégration position/rotation — vitesse * dt → Move / UpdateQuaternion.
//
//   6. Sleep                     — timer, seuils linéaire et angulaire.
//
//  Ordre : APRÈS PhysicSystem dans Phase::FixedUpdate.
// ─────────────────────────────────────────────────────────────────────────────
class PhysicIntegrateSystem : public System<RigidBodyComponent, MotionComponent,
                                            ShapeComponent,     TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override {}

    void OnUpdate(float _dt, EntityId _e,
                  RigidBodyComponent& _rigid,
                  MotionComponent&    _motion,
                  ShapeComponent&     _shape,
                  TransformComponent& _transform) override;

    void OnEndUpdate(float _dt) override {}

    void SetGravity(const XMFLOAT3& _g) { m_gravity = _g; }

private:
    void ComputeBodyInertiaTensor (RigidBodyComponent& _rigid, ShapeComponent& _shape);
    void UpdateWorldInertiaTensor (RigidBodyComponent& _rigid, TransformComponent& _transform);

    XMFLOAT3 IntegrateLinearVelocity  (RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt);
    XMFLOAT3 IntegrateAngularVelocity (RigidBodyComponent& _rigid, MotionComponent& _motion, float _dt);
    void     UpdateQuaternion         (TransformComponent& _transform, const XMFLOAT3& _deltaAngle);

    XMFLOAT3 m_gravity = { 0.0f, -9.81f, 0.0f };

    static constexpr float kSleepLinearThreshold  = 0.05f;
    static constexpr float kSleepAngularThreshold = 0.05f;
    static constexpr float kSleepTimeThreshold    = 0.5f;
    static constexpr float kMinAngularVelocitySq  = 1e-6f;
};

#endif // !PHYSIC_INTEGRATE_SYSTEM_H_DEFINED
