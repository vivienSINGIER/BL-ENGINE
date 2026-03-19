#include "PhysicSystem.h"
#include "Utils.hpp"

void PhysicSystem::OnStartUpdate(float _dt)
{
	m_gravityForce = XMFLOAT3(0.0f, -9.81f, 0.0f);
}

void PhysicSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
	if (_physic.type == BodyType::Static) return;

	//ResolveOverlap(_physic, _collider, _transform);

	XMFLOAT3 acceleration = Mul(_physic.forces, _physic.massInverse);

	if (_physic.useGravity)
		acceleration = Add(acceleration, m_gravityForce);

	XMFLOAT3 velocity = Add(_physic.velocity, Mul(acceleration, _dt));
	XMFLOAT3 move = Mul(velocity, _dt);

	_physic.acceleration = acceleration;
	_physic.velocity = velocity;
	_physic.forces = XMFLOAT3(0.0f, 0.0f, 0.0f);

	_transform.local.Move(move);
}

void PhysicSystem::OnEndUpdate(float _dt)
{
}

void PhysicSystem::ResolveOverlap(PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
	EntityId other = _collider.contact.other;
	if (other == -1) return;
	PhysicComponent& otherPhysic = world->GetComponent<PhysicComponent>(other);

	if (otherPhysic.type == BodyType::Static)
	{
		//Deplacement total si autre entité static
		XMFLOAT3 move = Mul(_collider.contact.normal, _collider.contact.penetration);
		_transform.local.SetPosition(Add(_transform.world.GetPosition(), move));
	}
	else
	{
		//Deplacement de moitié si autre entité dynamique
		XMFLOAT3 move = Mul(_collider.contact.normal, _collider.contact.penetration * 0.5f);
		_transform.local.Move(move);
	}
}
