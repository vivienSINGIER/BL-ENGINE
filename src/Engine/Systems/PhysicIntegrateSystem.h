#ifndef PHYSIC_INTEGRATE_SYSTEM_H_DEFINED
#define PHYSIC_INTEGRATE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/TransformComponent.hpp"
#include "../Components/PhysicComponent.hpp"
#include "../Components/ColliderComponent.hpp"

class PhysicIntegrateSystem : public System<PhysicComponent, ColliderComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetGravity(const XMFLOAT3& _gravity) { m_gravity = _gravity; }

private:
    //Tenseurs
    void ComputeBodyInertiaTensor(PhysicComponent& _physic, ColliderComponent& _collider);
    void UpdateWorldInertiaTensor(PhysicComponent& _physic, TransformComponent& _transform);

    // Intégration 
    XMFLOAT3 IntegrateLinearVelocity(PhysicComponent& _physic, float _dt);
    XMFLOAT3 IntegrateAngularVelocity(PhysicComponent& _physic, float _dt);
    void     UpdateQuaternion(TransformComponent& _transform, const XMFLOAT3& _deltaAngle);

    XMFLOAT3 m_gravity = { 0.0f, -9.81f, 0.0f };
};

#endif // !PHYSIC_INTEGRATE_SYSTEM_H_DEFINED