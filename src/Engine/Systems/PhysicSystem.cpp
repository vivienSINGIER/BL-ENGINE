#include "PhysicSystem.h"
#include "Utils.hpp"
#include <cmath>
#include <iostream>

namespace
{
    constexpr float kPenetrationSlop = 0.005f;
    constexpr float kPenetrationPercent = 0.8f;

    constexpr float kTangentEpsilonSq = 1e-6f;
    constexpr float kRestitutionThreshold = 0.2f;
    constexpr float kRestingNormalVelocityThreshold = 0.05f;
    constexpr float kRestingTangentVelocityThreshold = 0.01f;

    constexpr int kSolverIterations = 4;
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
    ResolveAllImpulses(kSolverIterations);
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

		WakeBodiesFromContact(physicA, physicB, contact);
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

            if (physicA.isSleeping && physicB.isSleeping)
                continue;

            for (int i = 0; i < contact.pointCount; ++i)
            {
                XMFLOAT3 point = contact.points[i].position;
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
        transformA.local.Move(Mul(_contact.normal, -moveAmountA));

    if (moveAmountB > 0.0f)
        transformB.local.Move(Mul(_contact.normal, moveAmountB));
}

void PhysicSystem::ResolveImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB,
    Contact& _contact, const XMFLOAT3& _point)
{
    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;

    if (invMassA + invMassB <= 0.0f) return;
        
    ContactPointContext ctx = BuildContactPointContext(_physicA, _physicB, _contact.a, _contact.b, _point);
    float velocityNormal = Dot(ctx.relativeVelocity, _contact.normal);

    if (velocityNormal >= 0.0f) return;

    // -----------------------------
    // Impulsion normale
    // -----------------------------
    float normalImpulseScalar = ComputeNormalImpulseScalar(_physicA, _physicB, _contact, ctx, _contact.pointCount);

    if (normalImpulseScalar <= 0.0f) return;

    XMFLOAT3 normalImpulse = Mul(_contact.normal, normalImpulseScalar);

    _physicA.velocity = Subtract(_physicA.velocity, Mul(normalImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(normalImpulse, invMassB));

    if (_physicA.type != BodyType::Static && _physicA.rotation)
    {
        XMFLOAT3 deltaAngularA = ComputeAngularVelocityDelta(ctx.rA, normalImpulse, _physicA.inertieInverse);
        _physicA.angularVelocity = Subtract(_physicA.angularVelocity, deltaAngularA);
    }

    if (_physicB.type != BodyType::Static && _physicB.rotation)
    {
        XMFLOAT3 deltaAngularB = ComputeAngularVelocityDelta(ctx.rB, normalImpulse, _physicB.inertieInverse);
        _physicB.angularVelocity = Add(_physicB.angularVelocity, deltaAngularB);
    }

    // -----------------------------
    // Recalcul au point après normale
    // -----------------------------
    ctx = BuildContactPointContext(_physicA, _physicB, _contact.a, _contact.b, _point);

    float postNormalVelocity = Dot(ctx.relativeVelocity, _contact.normal);
    XMFLOAT3 tangent = ComputeTangent(ctx.relativeVelocity, _contact.normal);

    if (Dot(tangent, tangent) < kTangentEpsilonSq)
        return;

    tangent = Normalize(tangent);
    float velocityTangent = Dot(ctx.relativeVelocity, tangent);

    // Contact quasi au repos : on ignore les micro-frictions parasites
    if (abs(velocityTangent) < 0.05f)
        return;
    if (abs(postNormalVelocity) < kRestingNormalVelocityThreshold && abs(velocityTangent) < kRestingTangentVelocityThreshold)
        return;

    // -----------------------------
    // Impulsion tangentielle
    // -----------------------------
    float tangentImpulseScalar = ComputeTangentImpulseScalar(_physicA, _physicB, _contact, ctx, tangent, _contact.pointCount);
    if (tangentImpulseScalar == 0.0f)
        return;

    float staticFriction = 0.5f * (_physicA.staticFriction + _physicB.staticFriction);
    float dynamicFriction = 0.5f * (_physicA.dynamicFriction + _physicB.dynamicFriction);

    XMFLOAT3 frictionImpulse;

    if (abs(tangentImpulseScalar) < normalImpulseScalar * staticFriction)
    {
        // friction statique
        frictionImpulse = Mul(tangent, tangentImpulseScalar);
    }
    else
    {
        // friction dynamique
        frictionImpulse = Mul(tangent, -normalImpulseScalar * dynamicFriction);
    }

    _physicA.velocity = Subtract(_physicA.velocity, Mul(frictionImpulse, invMassA));
    _physicB.velocity = Add(_physicB.velocity, Mul(frictionImpulse, invMassB));

    if (_physicA.type != BodyType::Static && _physicA.rotation)
    {
        XMFLOAT3 deltaAngularA = ComputeAngularVelocityDelta(ctx.rA, frictionImpulse, _physicA.inertieInverse);
        _physicA.angularVelocity = Subtract(_physicA.angularVelocity, deltaAngularA);
    }

    if (_physicB.type != BodyType::Static && _physicB.rotation)
    {
        XMFLOAT3 deltaAngularB = ComputeAngularVelocityDelta(ctx.rB, frictionImpulse, _physicB.inertieInverse);
        _physicB.angularVelocity = Add(_physicB.angularVelocity, deltaAngularB);
    }
}

PhysicSystem::ContactPointContext PhysicSystem::BuildContactPointContext(PhysicComponent& _physicA, PhysicComponent& _physicB,
    EntityId _entityA, EntityId _entityB, const XMFLOAT3& _point) const
{
    ContactPointContext ctx;
    ctx.point = _point;

    ctx.centerA = GetCenter(_entityA);
    ctx.centerB = GetCenter(_entityB);

    ctx.rA = Subtract(_point, ctx.centerA);
    ctx.rB = Subtract(_point, ctx.centerB);

    ctx.velocityAtPointA = Add(_physicA.velocity, Cross(_physicA.angularVelocity, ctx.rA));
    ctx.velocityAtPointB = Add(_physicB.velocity, Cross(_physicB.angularVelocity, ctx.rB));

    ctx.relativeVelocity = Subtract(ctx.velocityAtPointB, ctx.velocityAtPointA);

    return ctx;
}

float PhysicSystem::ComputeNormalImpulseScalar(PhysicComponent& _physicA, PhysicComponent& _physicB,
    const Contact& _contact, const ContactPointContext& _ctx, int _pointCount) const
{
    float velocityNormal = Dot(_ctx.relativeVelocity, _contact.normal);

    float restitution = Min(_physicA.restitution, _physicB.restitution);
    if (abs(velocityNormal) < kRestitutionThreshold)
        restitution = 0.0f;

    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;

    float normalMass =
        invMassA +
        invMassB +
        ComputeAngularEffectiveMassTerm(_ctx.rA, _contact.normal, _physicA.inertieInverse) +
        ComputeAngularEffectiveMassTerm(_ctx.rB, _contact.normal, _physicB.inertieInverse);

    if (normalMass <= 0.0f)
        return 0.0f;

    float impulse = -(1.0f + restitution) * velocityNormal / normalMass;
    impulse /= (float)_pointCount;

    return impulse;
}

float PhysicSystem::ComputeTangentImpulseScalar(PhysicComponent& _physicA, PhysicComponent& _physicB,
    const Contact& _contact, const ContactPointContext& _ctx, const XMFLOAT3& _tangent, int _pointCount) const
{
    float velocityTangent = Dot(_ctx.relativeVelocity, _tangent);

    float invMassA = _physicA.massInverse;
    float invMassB = _physicB.massInverse;

    float tangentMass =
        invMassA +
        invMassB +
        ComputeAngularEffectiveMassTerm(_ctx.rA, _tangent, _physicA.inertieInverse) +
        ComputeAngularEffectiveMassTerm(_ctx.rB, _tangent, _physicB.inertieInverse);

    if (tangentMass <= 0.0f)
        return 0.0f;

    float impulse = -velocityTangent / tangentMass;
    impulse /= (float)_pointCount;

    return impulse;
}

XMFLOAT3 PhysicSystem::ComputeTangent(const XMFLOAT3& _relativeVelocity, const XMFLOAT3& _normal) const
{
    return Subtract(_relativeVelocity, Mul(_normal, Dot(_relativeVelocity, _normal)));
}

XMFLOAT3 PhysicSystem::ComputeAngularVelocityDelta(const XMFLOAT3& _r, const XMFLOAT3& _impulse, const XMFLOAT3& _inertiaInverse) const
{
    XMFLOAT3 angularImpulse = Cross(_r, _impulse);
    return ApplyInertiaInverse(angularImpulse, _inertiaInverse);
}

XMFLOAT3 PhysicSystem::ApplyInertiaInverse(const XMFLOAT3& _v, const XMFLOAT3& _inertiaInverse) const
{
    return
    {
        _v.x * _inertiaInverse.x,
        _v.y * _inertiaInverse.y,
        _v.z * _inertiaInverse.z
    };
}

float PhysicSystem::ComputeAngularEffectiveMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const XMFLOAT3& _inertiaInverse) const
{
    XMFLOAT3 rxn = Cross(_r, _axis);
    XMFLOAT3 i_rxn = ApplyInertiaInverse(rxn, _inertiaInverse);
    XMFLOAT3 crossTerm = Cross(i_rxn, _r);
    return Dot(crossTerm, _axis);
}

XMFLOAT3 PhysicSystem::GetCenter(EntityId _e) const
{
    TransformComponent& transform = world->GetComponent<TransformComponent>(_e);
    return transform.local.GetPosition();
}

XMFLOAT3 PhysicSystem::GetVelocityAtPoint(const PhysicComponent& _physic, EntityId _e, const XMFLOAT3& _point) const
{
    XMFLOAT3 center = GetCenter(_e);
    XMFLOAT3 r = Subtract(_point, center);
    XMFLOAT3 angularContribution = Cross(_physic.angularVelocity, r);
    return Add(_physic.velocity, angularContribution);
}

bool PhysicSystem::isStableSupport(const PhysicComponent& _physic)
{
    if (_physic.type == BodyType::Static) return true;
    if (_physic.isSleeping) return true;
    if (_physic.hasSupportContact == false) return false;

    return false;
}

void PhysicSystem::WakeBodiesFromContact(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact)
{
    // Gestion support
    if (_contact.normal.y < -0.5f)
    {
        _physicA.hasSupportContact = true;
        _physicA.supportNormal = Inverse(_contact.normal);
    }

    if (_contact.normal.y > 0.5f)
    {
        _physicB.hasSupportContact = true;
        _physicB.supportNormal = _contact.normal;
    }

    // Réveil uniquement sur impact significatif
    XMFLOAT3 relativeVelocity = Subtract(_physicB.velocity, _physicA.velocity);
    float velocityNormal = Dot(relativeVelocity, _contact.normal);

    // si velocityNormal < 0 -> ils se rapprochent
    float wakeImpactThreshold = 0.3f;

    if (velocityNormal < -wakeImpactThreshold)
    {
        _physicA.WakeUp();
        _physicB.WakeUp();
    }
}