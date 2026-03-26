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
    ResolveAllImpulses(4);
}

void PhysicSystem::ResolveAllOverlaps()
{
    for (Contact& contact : m_pContactManager->contacts)
    {
        if (!world->HasComponent<PhysicComponent>(contact.a))
            continue;
        if (!world->HasComponent<PhysicComponent>(contact.b))
            continue;

        PhysicComponent& physicA = world->GetComponent<PhysicComponent>(contact.a);
        PhysicComponent& physicB = world->GetComponent<PhysicComponent>(contact.b);

        ResolveOverlap(physicA, physicB, contact);
    }
}


void PhysicSystem::ResolveAllImpulses(int _iterations)
{
    for (int iteration = 0; iteration < _iterations; ++iteration)
    {
        for (Contact& contact : m_pContactManager->contacts)
        {
            if (!world->HasComponent<PhysicComponent>(contact.a))
                continue;
            if (!world->HasComponent<PhysicComponent>(contact.b))
                continue;

            PhysicComponent& physicA = world->GetComponent<PhysicComponent>(contact.a);
            PhysicComponent& physicB = world->GetComponent<PhysicComponent>(contact.b);

            int pointCount = contact.pointCount;

            // fallback si tu gardes encore un contact.point unique
            if (pointCount <= 0)
            {
                ResolveImpulseAtPoint(physicA, physicB, contact, contact.points[0].position);
                continue;
            }

            for (int i = 0; i < pointCount; ++i)
            {
                ResolveImpulseAtPoint(physicA, physicB, contact, contact.points[i].position);
            }
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

void PhysicSystem::ResolveImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point)
{
    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;
    float totalInvMass = invMassA + invMassB;

    if (totalInvMass <= 0.0f)
        return;

    XMFLOAT3 relativeVelocity = Subtract(_physicB.velocity, _physicA.velocity);
    float velocityNormal = Dot(relativeVelocity, _contact.normal);

    if (velocityNormal >= 0.0f)
        return;

    float e = Min(_physicA.restitution, _physicB.restitution);

    // Repartition de l'impulsion normale sur les points
    int pointCount = (_contact.pointCount > 0) ? _contact.pointCount : 1;

    float impulseScalar = -(1.0f + e) * velocityNormal / totalInvMass;
    impulseScalar /= static_cast<float>(pointCount);

    XMFLOAT3 normalImpulse = Mul(_contact.normal, impulseScalar);

    _physicA.velocity = Subtract(_physicA.velocity, Mul(normalImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(normalImpulse, invMassB));

    // vitesse relative après impulsion normale
    relativeVelocity = Subtract(_physicB.velocity, _physicA.velocity);

    XMFLOAT3 tangent = Subtract(relativeVelocity,
        Mul(_contact.normal, Dot(relativeVelocity, _contact.normal)));

    float tangentLenSq = Dot(tangent, tangent);
    if (tangentLenSq < 1e-6f)
        return;

    tangent = Normalize(tangent);

    float velocityTangent = Dot(relativeVelocity, tangent);
    float impulseTangentScalar = -velocityTangent / totalInvMass;
    impulseTangentScalar /= static_cast<float>(pointCount);

    float staticFriction = 0.5f * (_physicA.staticFriction + _physicB.staticFriction);
    float dynamicFriction = 0.5f * (_physicA.dynamicFriction + _physicB.dynamicFriction);

    XMFLOAT3 frictionImpulse;
    if (abs(impulseTangentScalar) < impulseScalar * staticFriction) //Satique
    {
        frictionImpulse = Mul(tangent, impulseTangentScalar);
    }
	else // Dynamique
    {
        frictionImpulse = Mul(tangent, -impulseScalar * dynamicFriction);
        CalculateTorqueAtPoint(_physicA, _physicB, _contact, _point, frictionImpulse);
    }

    _physicA.velocity = Subtract(_physicA.velocity, Mul(frictionImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(frictionImpulse, invMassB));

    std::cout << "velocityNormal = " << velocityNormal << "\n";
    std::cout << "velocityTangent = " << velocityTangent << "\n";
    std::cout << "normal = " << _contact.normal.x << ", " << _contact.normal.y << ", " << _contact.normal.z << "\n";
}

void PhysicSystem::CalculateTorqueAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point, XMFLOAT3& _impulse)
{
    TransformComponent& transformA = world->GetComponent<TransformComponent>(_contact.a);
    TransformComponent& transformB = world->GetComponent<TransformComponent>(_contact.b);

    XMFLOAT3 centerA = transformA.world.GetPosition();
    XMFLOAT3 centerB = transformB.world.GetPosition();

    XMFLOAT3 rA = Subtract(_point, centerA);
    XMFLOAT3 rB = Subtract(_point, centerB);

    XMFLOAT3 torqueA = Cross(rA, Inverse(_impulse));
    XMFLOAT3 torqueB = Cross(rB, _impulse);

    torqueA = Mul(torqueA, _physicA.inertieInverse);
    torqueB = Mul(torqueB, _physicB.inertieInverse);

    if (_physicA.rotation)
        _physicA.angularVelocity = Add(_physicA.angularVelocity, torqueA);

    if (_physicB.rotation)
        _physicB.angularVelocity = Add(_physicB.angularVelocity, torqueB);
}