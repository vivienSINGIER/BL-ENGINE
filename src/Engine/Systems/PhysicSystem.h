#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/PhysicComponent.hpp"
#include "../ContactManager.hpp"

class PhysicSystem : public System<PhysicComponent, TransformComponent>
{
public:
	void OnStartUpdate(float _dt) override;
	void OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform) override;
	void OnEndUpdate(float _dt) override;

	void SetContactManager(ContactManager* _contactManager) { m_pContactManager = _contactManager; }

private:
    void ResolveAllOverlaps();
    void ResolveAllImpulses(int _iterations = 4);

    void ResolveOverlap(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact);

    void ResolveImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point);
    void CalculateTorqueAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point, XMFLOAT3& _impulse);

    ContactManager* m_pContactManager = nullptr;
};

#endif // !PHYSIC_SYSTEM_H_DEFINED