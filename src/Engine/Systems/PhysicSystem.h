#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/PhysicComponent.hpp"

class PhysicSystem : public System<PhysicComponent, ColliderComponent, TransformComponent>
{
public:
	void OnStartUpdate(float _dt) override;
	void OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform) override;
	void OnEndUpdate(float _dt) override;

private:
	void ResolveOverlap(PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform);

	XMFLOAT3 m_gravityAccel = XMFLOAT3(0.0f, -9.81f, 0.0f);
};

#endif // !PHYSIC_SYSTEM_H_DEFINED