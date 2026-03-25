#ifndef PHYSIC_INTEGRATE_SYSTEM_H_DEFINE
#define PHYSIC_INTEGRATE_SYSTEM_H_DEFINE

#include "../ECS/ISystem.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/PhysicComponent.hpp"

class PhysicIntegrateSystem : public System<PhysicComponent, TransformComponent>
{
public:
	void OnStartUpdate(float _dt) override;
	void OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform) override;
	void OnEndUpdate(float _dt) override;

private:
	float m_airDrag = 0.3f;
	XMFLOAT3 m_gravityAccel = XMFLOAT3(0.0f, -9.81f, 0.0f);
};

#endif // !PHYSIC_INTEGRATE_SYSTEM_H_DEFINEd
