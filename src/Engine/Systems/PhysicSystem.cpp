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
    float velocityNormal = Dot(relativeVelocity, _contact.normal);

    // Déjà en train de s'éloigner
    if (velocityNormal >= 0.0f)
        return;

    float e = Min(_physicA.restitution, _physicB.restitution);

    // Impulsion normale
    float impulse = -(1.0f + e) * velocityNormal / totalInvMass;
    XMFLOAT3 normalImpulse = Mul(_contact.normal, impulse);

    _physicA.velocity = Subtract(_physicA.velocity, Mul(normalImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(normalImpulse, invMassB));

	CalculateTorque(_physicA, _physicB, _contact, normalImpulse);

    // Recalculer la vitesse relative après l'impulsion normale
    relativeVelocity = Subtract(_physicB.velocity, _physicA.velocity);

    // Tangente
    XMFLOAT3 tangent = Subtract(relativeVelocity, Mul(_contact.normal, Dot(relativeVelocity, _contact.normal)));
    float tangentLenSq = Dot(tangent, tangent);

	if (tangentLenSq < 1e-6f) //Si la tangente est trop petite
        return;

    tangent = Normalize(tangent);

    float velocityTangent = Dot(relativeVelocity, tangent);

    // Impulsion tangente
    float impusleTangent = -velocityTangent / totalInvMass;

    float staticFriction = 0.5f * (_physicA.staticFriction + _physicB.staticFriction);
    float dynamicFriction = 0.5f * (_physicA.dynamicFriction + _physicB.dynamicFriction);

    XMFLOAT3 frictionImpulse;

    // Friction statique / dynamique
    if (abs(impusleTangent) < impulse * staticFriction)
    {
        // Friction statique : annule complètement le mouvement tangent
        frictionImpulse = Mul(tangent, impusleTangent);
    }
    else
    {
        // Friction dynamique : limite l'impulsion tangentielle
        frictionImpulse = Mul(tangent, -impulse * dynamicFriction);
    }

    _physicA.velocity = Subtract(_physicA.velocity, Mul(frictionImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(frictionImpulse, invMassB));
}

void PhysicSystem::CalculateTorque(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _nImpulse)
{
    TransformComponent& transformA = world->GetComponent<TransformComponent>(_contact.a);
    TransformComponent& transformB = world->GetComponent<TransformComponent>(_contact.b);

    XMFLOAT3 centerA = transformA.world.GetPosition();
    XMFLOAT3 centerB = transformB.world.GetPosition();

    XMFLOAT3 rA = Subtract(_contact.point, centerA);
    XMFLOAT3 rB = Subtract(_contact.point, centerB);

    XMFLOAT3 torqueA = Cross(rA, Inverse(_nImpulse));
    XMFLOAT3 torqueB = Cross(rB, _nImpulse);

	torqueA = Mul(torqueA, _physicA.inertieInverse);
	torqueB = Mul(torqueB, _physicB.inertieInverse);

    if (_physicA.rotation)
        _physicA.angularVelocity = Add(_physicA.angularVelocity, torqueA);

    if (_physicB.rotation)
        _physicB.angularVelocity = Add(_physicB.angularVelocity, torqueB);
}