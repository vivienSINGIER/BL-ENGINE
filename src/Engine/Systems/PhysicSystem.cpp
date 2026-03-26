#include "PhysicSystem.h"
#include "Utils.hpp"
#include <cmath>

namespace
{
    constexpr float kPenetrationSlop = 0.001f;
    constexpr float kPenetrationPercent = 0.2f;
    constexpr float kTangentEpsilonSq = 1e-6f;
}

void PhysicSystem::OnStartUpdate(float _dt)
{
}

void PhysicSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform)
{
}

void PhysicSystem::OnEndUpdate(float _dt)
{
    if (m_pContactManager == nullptr)
        return;

    ResolveAllOverlaps();
    ResolveAllImpulses(4);
}

void PhysicSystem::ResolveAllOverlaps()
{
    for (Contact& contact : m_pContactManager->contacts)
    {
        if (world->HasComponent<PhysicComponent>(contact.a) == false ||
            world->HasComponent<PhysicComponent>(contact.b) == false)
        {
            continue;
        }

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
            if (world->HasComponent<PhysicComponent>(contact.a) == false ||
                world->HasComponent<PhysicComponent>(contact.b) == false)
            {
                continue;
            }

            PhysicComponent& physicA = world->GetComponent<PhysicComponent>(contact.a);
            PhysicComponent& physicB = world->GetComponent<PhysicComponent>(contact.b);

            int pointCount = (contact.pointCount > 0) ? contact.pointCount : 1;

            for (int i = 0; i < pointCount; ++i)
            {
                XMFLOAT3 point = (contact.pointCount > 0) ? contact.points[i].position : contact.points[0].position;
                ResolveImpulseAtPoint(physicA, physicB, contact, point);
            }
        }
    }
}

void PhysicSystem::ResolveOverlap(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact)
{
    if (_contact.penetration <= 0.0f)
        return;

    float correctionDepth = Max(0.0f, (_contact.penetration - kPenetrationSlop) * kPenetrationPercent);
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

    TransformComponent& transformA = world->GetComponent<TransformComponent>(_contact.a);
    TransformComponent& transformB = world->GetComponent<TransformComponent>(_contact.b);

    if (moveAmountA > 0.0f)
    {
        XMFLOAT3 moveA = Mul(_contact.normal, -moveAmountA);
        transformA.local.Move(moveA);
    }

    if (moveAmountB > 0.0f)
    {
        XMFLOAT3 moveB = Mul(_contact.normal, moveAmountB);
        transformB.local.Move(moveB);
    }
}

void PhysicSystem::ResolveImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point)
{
    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;

    if (invMassA + invMassB <= 0.0f)
        return;

    XMFLOAT3 centerA = GetCenterWorld(_contact.a);
    XMFLOAT3 centerB = GetCenterWorld(_contact.b);

    XMFLOAT3 rA = Subtract(_point, centerA);
    XMFLOAT3 rB = Subtract(_point, centerB);

    // Vitesse complète au point de contact
    XMFLOAT3 velocityAtPointA = GetVelocityAtPoint(_physicA, _contact.a, _point);
    XMFLOAT3 velocityAtPointB = GetVelocityAtPoint(_physicB, _contact.b, _point);
    XMFLOAT3 relativeVelocity = Subtract(velocityAtPointB, velocityAtPointA);

    float velocityNormal = Dot(relativeVelocity, _contact.normal);
    if (velocityNormal >= 0.0f)
        return;

    int pointCount = (_contact.pointCount > 0) ? _contact.pointCount : 1;
    float restitution = Min(_physicA.restitution, _physicB.restitution);

    // Masse effective sur la normale
    float normalMass =
        invMassA +
        invMassB +
        ComputeAngularEffectiveMassTerm(rA, _contact.normal, _physicA.inertieInverse) +
        ComputeAngularEffectiveMassTerm(rB, _contact.normal, _physicB.inertieInverse);

    if (normalMass <= 0.0f)
        return;

    float normalImpulseScalar = -(1.0f + restitution) * velocityNormal / normalMass;
    normalImpulseScalar /= static_cast<float>(pointCount);

    XMFLOAT3 normalImpulse = Mul(_contact.normal, normalImpulseScalar);

    // Impulsion linéaire
    _physicA.velocity = Subtract(_physicA.velocity, Mul(normalImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(normalImpulse, invMassB));

    // Impulsion angulaire liée à la normale
    if (_physicA.rotation)
    {
        XMFLOAT3 nomalImpulseInverse = Inverse(normalImpulse);
		XMFLOAT3 value = Cross(rA, nomalImpulseInverse);
        XMFLOAT3 deltaAngularA = ApplyInertiaInverse(value, _physicA.inertieInverse);
        _physicA.angularVelocity = Add(_physicA.angularVelocity, deltaAngularA);
    }

    if (_physicB.rotation)
    {
        XMFLOAT3 value = Cross(rB, normalImpulse);
        XMFLOAT3 deltaAngularB = ApplyInertiaInverse(value, _physicB.inertieInverse);
        _physicB.angularVelocity = Add(_physicB.angularVelocity, deltaAngularB);
    }

    // Recalcul de la vitesse relative après impulsion normale
    velocityAtPointA = GetVelocityAtPoint(_physicA, _contact.a, _point);
    velocityAtPointB = GetVelocityAtPoint(_physicB, _contact.b, _point);
    relativeVelocity = Subtract(velocityAtPointB, velocityAtPointA);

    XMFLOAT3 tangent = Subtract(relativeVelocity,
        Mul(_contact.normal, Dot(relativeVelocity, _contact.normal)));

    const float tangentLenSq = Dot(tangent, tangent);
    if (tangentLenSq < kTangentEpsilonSq)
        return;

    tangent = Normalize(tangent);

    const float velocityTangent = Dot(relativeVelocity, tangent);

    float tangentMass =
        invMassA +
        invMassB +
        ComputeAngularEffectiveMassTerm(rA, tangent, _physicA.inertieInverse) +
        ComputeAngularEffectiveMassTerm(rB, tangent, _physicB.inertieInverse);

    if (tangentMass <= 0.0f)
        return;

    float tangentImpulseScalar = -velocityTangent / tangentMass;
    tangentImpulseScalar /= static_cast<float>(pointCount);

    const float staticFriction = 0.5f * (_physicA.staticFriction + _physicB.staticFriction);
    const float dynamicFriction = 0.5f * (_physicA.dynamicFriction + _physicB.dynamicFriction);

    XMFLOAT3 frictionImpulse;

    if (abs(tangentImpulseScalar) < normalImpulseScalar * staticFriction)
    {
        frictionImpulse = Mul(tangent, tangentImpulseScalar);
    }
    else
    {
        frictionImpulse = Mul(tangent, -normalImpulseScalar * dynamicFriction);
    }

    // Impulsion linéaire de friction
    _physicA.velocity = Subtract(_physicA.velocity, Mul(frictionImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(frictionImpulse, invMassB));

    // Impulsion angulaire de friction
    if (_physicA.rotation)
    {
		XMFLOAT3 value = Cross(rA, Inverse(frictionImpulse));
        XMFLOAT3 deltaAngularA = ApplyInertiaInverse(value, _physicA.inertieInverse);
        _physicA.angularVelocity = Add(_physicA.angularVelocity, deltaAngularA);
    }

    if (_physicB.rotation)
    {
		XMFLOAT3 value = Cross(rB, frictionImpulse);
        XMFLOAT3 deltaAngularB = ApplyInertiaInverse(value, _physicB.inertieInverse);
        _physicB.angularVelocity = Add(_physicB.angularVelocity, deltaAngularB);
    }
}

void PhysicSystem::ApplyAngularImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact, XMFLOAT3& _point, XMFLOAT3& _impulse)
{
    XMFLOAT3 centerA = GetCenterWorld(_contact.a);
    XMFLOAT3 centerB = GetCenterWorld(_contact.b);

    XMFLOAT3 rA = Subtract(_point, centerA);
    XMFLOAT3 rB = Subtract(_point, centerB);

    XMFLOAT3 torqueA = Mul(Cross(rA, Inverse(_impulse)), _physicA.inertieInverse);
    XMFLOAT3 torqueB = Mul(Cross(rB, _impulse), _physicB.inertieInverse);

    if (_physicA.rotation)
        _physicA.angularVelocity = Add(_physicA.angularVelocity, torqueA);

    if (_physicB.rotation)
        _physicB.angularVelocity = Add(_physicB.angularVelocity, torqueB);
}

XMFLOAT3 PhysicSystem::ApplyInertiaInverse(XMFLOAT3& v, XMFLOAT3& inertiaInverse)
{
    return
    {
        v.x * inertiaInverse.x,
        v.y * inertiaInverse.y,
        v.z * inertiaInverse.z
    };
}

float PhysicSystem::ComputeAngularEffectiveMassTerm(XMFLOAT3& r, XMFLOAT3& axis, XMFLOAT3& inertiaInverse)
{
    XMFLOAT3 rxn = Cross(r, axis);
    XMFLOAT3 i_rxn = ApplyInertiaInverse(rxn, inertiaInverse);
    XMFLOAT3 crossTerm = Cross(i_rxn, r);
    return Dot(crossTerm, axis);
}

XMFLOAT3 PhysicSystem::GetCenterWorld(EntityId _e) 
{
    TransformComponent& transform = world->GetComponent<TransformComponent>(_e);
    return transform.world.GetPosition();
}

XMFLOAT3 PhysicSystem::GetVelocityAtPoint(PhysicComponent& _physic, EntityId _e, XMFLOAT3& _point)
{
    XMFLOAT3 center = GetCenterWorld(_e);
    XMFLOAT3 r = Subtract(_point, center);
    XMFLOAT3 angularContribution = Cross(_physic.angularVelocity, r);
    return Add(_physic.velocity, angularContribution);
}