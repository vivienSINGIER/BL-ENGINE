#include "PhysicSystem.h"
#include "Utils.hpp"

void PhysicSystem::OnStartUpdate(float _dt)
{

}

void PhysicSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
	if (_physic.type == BodyType::Static) return;

	ResolveOverlap(_physic, _collider, _transform);

	float velNormAxe = Dot(_physic.velocity, _collider.contact.normal);
	if (velNormAxe < 0.0f)
	{
		_physic.velocity = Subtract(_physic.velocity, Mul(_collider.contact.normal, velNormAxe));
	}

	XMFLOAT3 acceleration = Mul(_physic.forces, _physic.massInverse);

	if (_physic.useGravity)
		acceleration = Add(acceleration, m_gravityAccel);

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
	float invMassA = _physic.massInverse;
	float invMassB = otherPhysic.massInverse;
	float totalInvMass = invMassA + invMassB;

	float slop = 0.001f; 
	float correctionDepth = Max(0.0f, _collider.contact.penetration - slop);

	XMFLOAT3 move = { 0.0f, 0.0f, 0.0f };
	float moveAmount = 0.0f;

	if (otherPhysic.type == BodyType::Static)
	{
		//Deplacement total si autre entité static
		moveAmount = _collider.contact.penetration;
	}
	else
	{
		//Deplacement de moitié si autre entité dynamique
		moveAmount = _collider.contact.penetration * invMassA / totalInvMass;
	}

	move = Mul(_collider.contact.normal, moveAmount);
	//move = Mul(move, correctionDepth);
	_transform.local.Move(move);
}
