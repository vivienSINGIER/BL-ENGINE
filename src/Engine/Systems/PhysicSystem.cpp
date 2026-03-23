#include "PhysicSystem.h"
#include "Utils.hpp"
#include <iostream>

void PhysicSystem::OnStartUpdate(float _dt)
{ 
}

void PhysicSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform)
{
}

void PhysicSystem::OnEndUpdate(float _dt)
{
	ResolveAllOverlaps();
	ResolveAllImpulses();
}

void PhysicSystem::ResolveAllOverlaps()
{
    for (Contact& contact : m_pContactManager->contacts)
    {
        if (world->HasComponent<PhysicComponent>(contact.a) == false)
            continue;
        if (world->HasComponent<PhysicComponent>(contact.b) == false)
            continue;

        PhysicComponent& physicA = world->GetComponent<PhysicComponent>(contact.a);
        PhysicComponent& physicB = world->GetComponent<PhysicComponent>(contact.b);

        ResolveOverlap(physicA, physicB, contact);
    }
}

void PhysicSystem::ResolveAllImpulses()
{
    for (int iteration = 0; iteration < 4; ++iteration)
    {
        for (Contact& contact : m_pContactManager->contacts)
        {
            if (world->HasComponent<PhysicComponent>(contact.a) == false)
                continue;
            if (world->HasComponent<PhysicComponent>(contact.b) == false)
                continue;

            PhysicComponent& physicA = world->GetComponent<PhysicComponent>(contact.a);
            PhysicComponent& physicB = world->GetComponent<PhysicComponent>(contact.b);

            ResolveImpulse(physicA, physicB, contact);
        }
    }
}

void PhysicSystem::ResolveOverlap(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact)
{
    if (_contact.penetration <= 0.0f)
        return;

    float slop = 0.001f;
    float percent = 0.2f;
    float correctionDepth = Max(0.0f, (_contact.penetration - slop) * percent);

    if (correctionDepth <= 0.0f)
        return;

    float moveAmountA = 0.0f;
    float moveAmountB = 0.0f;

    if (_physicB.type == BodyType::Static)
    {
        moveAmountA = correctionDepth;
    }
    else if (_physicA.type == BodyType::Static)
    {
        moveAmountB = correctionDepth;
    }
    else
    {
        float invMassA = _physicA.massInverse;
        float invMassB = _physicB.massInverse;
        float totalInvMass = invMassA + invMassB;

        if (totalInvMass <= 0.0f)
            return;

        moveAmountA = correctionDepth * (invMassA / totalInvMass);
        moveAmountB = correctionDepth * (invMassB / totalInvMass);
    }

    XMFLOAT3 moveA = Mul(_contact.normal, -moveAmountA);
    XMFLOAT3 moveB = Mul(_contact.normal, moveAmountB);

    TransformComponent& transformA = world->GetComponent<TransformComponent>(_contact.a);
    TransformComponent& transformB = world->GetComponent<TransformComponent>(_contact.b);

    if (moveAmountA > 0.0f)
        transformA.local.Move(moveA);

    if (moveAmountB > 0.0f)
        transformB.local.Move(moveB);
}

void PhysicSystem::ResolveImpulse(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact)
{
    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return;

    XMFLOAT3 relativeVelocity = Subtract(_physicB.velocity, _physicA.velocity);
    float vn = Dot(relativeVelocity, _contact.normal);

    // Déjà en train de s'éloigner
    if (vn >= 0.0f)
        return;

    float e = Min(_physicA.restitution, _physicB.restitution);

    float j = -(1.0f + e) * vn / totalInvMass;
    XMFLOAT3 impulse = Mul(_contact.normal, j);

    _physicA.velocity = Subtract(_physicA.velocity, Mul(impulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(impulse, invMassB));
}
