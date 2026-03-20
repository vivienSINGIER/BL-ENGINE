#include "PhysicSystem.h"
#include "Utils.hpp"

void PhysicSystem::OnStartUpdate(float _dt)
{

}

void PhysicSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
	if (_physic.type == BodyType::Static) return;

	ResolveContacts(_physic, _collider, _transform);

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

void PhysicSystem::ResolveContacts(PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
	for (int i = 0; i < _collider.contactCount; i++)
	{
		Contact& contact = _collider.contact[i];
		PhysicComponent& otherPhysic = world->GetComponent<PhysicComponent>(contact.other);

		_transform.local.Move(ResolveOverlap(_physic, otherPhysic, contact));

		float velNormAxe = Dot(_physic.velocity, contact.normal);
		if (velNormAxe < 0.0f)
		{
			_physic.velocity = Subtract(_physic.velocity, Mul(contact.normal, velNormAxe));
		}
	}
	_collider.contactCount = 0;
}

XMFLOAT3 PhysicSystem::ResolveOverlap(PhysicComponent& _physic, PhysicComponent& _otherPhysic, Contact& _contact)
{
	float invMassA = _physic.massInverse;
	float invMassB = _otherPhysic.massInverse;
	float totalInvMass = invMassA + invMassB;

	float slop = 0.0001f; 
	float correctionDepth = Max(0.0f, _contact.penetration - slop);

	XMFLOAT3 move = { 0.0f, 0.0f, 0.0f };
	float moveAmount = 0.0f;

	if (_otherPhysic.type == BodyType::Static)
	{
		//Deplacement total si autre entité static
		moveAmount = _contact.penetration;
	}
	else
	{
		//Deplacement de moitié si autre entité dynamique
		moveAmount = _contact.penetration * invMassA / totalInvMass;
	}

	move = Mul(_contact.normal, moveAmount);
	move = Mul(move, correctionDepth);
	return move;
}
