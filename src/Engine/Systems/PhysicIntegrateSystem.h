#ifndef PHYSIC_INTEGRATE_SYSTEM_H_DEFINE
#define PHYSIC_INTEGRATE_SYSTEM_H_DEFINE

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

private:
	XMFLOAT3 IntegrateVelocity(PhysicComponent& _physic, float _dt);
	XMFLOAT3 IntegrateTorque(PhysicComponent& _physic, float _dt);

	void UpdateQuaternion(TransformComponent& _transform, XMFLOAT3& deltaAngle);

	void BoxInertie(PhysicComponent& _physic, ColliderComponent& _collider);
	void SphereInertie(PhysicComponent& _physic, ColliderComponent& _collider);

	float m_airDrag = 0.3f;
	XMFLOAT3 m_gravityAccel = XMFLOAT3(0.0f, -9.81f, 0.0f);
};

#endif // !PHYSIC_INTEGRATE_SYSTEM_H_DEFINEd
