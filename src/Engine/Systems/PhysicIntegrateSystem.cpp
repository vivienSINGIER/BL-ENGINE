#include "PhysicIntegrateSystem.h"
#include <iostream>

namespace
{
    constexpr float kSleepLinearThreshold = 0.05f;
    constexpr float kSleepAngularThreshold = 0.05f;
	constexpr float kSleepTimeThreshold = 0.5f;
}

void PhysicIntegrateSystem::OnStartUpdate(float _dt)
{
}

void PhysicIntegrateSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, ColliderComponent& _collider, TransformComponent& _transform)
{
    if (_physic.type == BodyType::Static)
        return;
	if (_physic.isSleeping)
		return;

    _transform.local.Move(IntegrateVelocity(_physic, _dt));

    if (_collider.type == ColliderType::Box)
        BoxInertie(_physic, _collider);
    else
        SphereInertie(_physic, _collider);

    XMFLOAT3 deltaAngle = IntegrateTorque(_physic, _dt);
    UpdateQuaternion(_transform, deltaAngle);

    // Sleep logic
    XMFLOAT3 velocityForSleep = _physic.velocity;

    if (_physic.hasSupportContact)
    {
        XMFLOAT3 n = Normalize(_physic.supportNormal);
        float vn = Dot(velocityForSleep, n);

        velocityForSleep = Subtract(velocityForSleep, Mul(n, vn));

        if (abs(vn) < 0.1f)
            _physic.velocity = Subtract(_physic.velocity, Mul(n, vn));
    }

    float linearSq = NormSquared(velocityForSleep);
    float angularSq = NormSquared(_physic.angularVelocity);

    float linearThresholdSq = kSleepLinearThreshold * kSleepLinearThreshold;
    float angularThresholdSq = kSleepAngularThreshold * kSleepAngularThreshold;

    bool lowMotion =
        linearSq < linearThresholdSq &&
        angularSq < angularThresholdSq;

    if (lowMotion && _physic.hasSupportContact)
    {
        _physic.sleepTimer += _dt;

        if (_physic.sleepTimer >= kSleepTimeThreshold)
        {
            std::cout << "Entity " << _e << " is sleeping." << std::endl;
            _physic.Sleep();
        }
    }
    else
    {
        _physic.sleepTimer = 0.0f;
    }

    _physic.hasSupportContact = false;
	_physic.supportNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void PhysicIntegrateSystem::OnEndUpdate(float _dt)
{
}

XMFLOAT3 PhysicIntegrateSystem::IntegrateVelocity(PhysicComponent& _physic, float _dt)
{
    XMFLOAT3 dragForce = Mul(_physic.velocity, -m_airDrag);
    XMFLOAT3 totalForces = Add(_physic.forces, dragForce);
    XMFLOAT3 acceleration = Mul(totalForces, _physic.massInverse);

    if (_physic.useGravity)
        acceleration = Add(acceleration, m_gravityAccel);

    _physic.acceleration = acceleration;
    _physic.velocity = Add(_physic.velocity, Mul(acceleration, _dt));
    _physic.forces = XMFLOAT3(0.0f, 0.0f, 0.0f);

    return Mul(_physic.velocity, _dt);
}

XMFLOAT3 PhysicIntegrateSystem::IntegrateTorque(PhysicComponent& _physic, float _dt)
{
    XMFLOAT3 angularAcceleration =
    {
        _physic.torque.x * _physic.inertieInverse.x,
        _physic.torque.y * _physic.inertieInverse.y,
        _physic.torque.z * _physic.inertieInverse.z
    };

    _physic.angularVelocity = Add(_physic.angularVelocity, Mul(angularAcceleration, _dt));

    float angularDampingFactor = 1.0f / (1.0f + _physic.angularDamping * _dt);
    _physic.angularVelocity = Mul(_physic.angularVelocity, angularDampingFactor);
    _physic.torque = XMFLOAT3(0.0f, 0.0f, 0.0f);

    return Mul(_physic.angularVelocity, _dt); //Angle delta
}

void PhysicIntegrateSystem::UpdateQuaternion(TransformComponent& _transform, XMFLOAT3& deltaAngle)
{
    XMVECTOR qCurrent = XMLoadFloat4(&_transform.local.GetRotation());
    XMVECTOR qDelta = XMQuaternionRotationRollPitchYaw(deltaAngle.x, deltaAngle.y, deltaAngle.z);
    XMVECTOR qNew = XMQuaternionNormalize(XMQuaternionMultiply(qDelta, qCurrent));

    XMFLOAT4 out;
    XMStoreFloat4(&out, qNew);
    _transform.local.SetRotationQuaternion(out);
}

void PhysicIntegrateSystem::BoxInertie(PhysicComponent& _physic, ColliderComponent& _collider)
{
	OBB& obb = _collider.obb;

    float w = obb.halfExtents.x * 2.0f;
    float h = obb.halfExtents.y * 2.0f;
    float d = obb.halfExtents.z * 2.0f;
    float m = _physic.mass;

    _physic.inertie.x = (1.0f / 12.0f) * m * (h * h + d * d);
    _physic.inertie.y = (1.0f / 12.0f) * m * (w * w + d * d);
    _physic.inertie.z = (1.0f / 12.0f) * m * (w * w + h * h);

    _physic.inertieInverse.x = (_physic.inertie.x > 0.0f) ? 1.0f / _physic.inertie.x : 0.0f;
    _physic.inertieInverse.y = (_physic.inertie.y > 0.0f) ? 1.0f / _physic.inertie.y : 0.0f;
    _physic.inertieInverse.z = (_physic.inertie.z > 0.0f) ? 1.0f / _physic.inertie.z : 0.0f;
}

void PhysicIntegrateSystem::SphereInertie(PhysicComponent& _physic, ColliderComponent& _collider)
{
	float radius = _collider.colliderTransform.GetScale().x * 0.5f;

    float i = (2.0f / 5.0f) * _physic.mass * radius * radius;

    _physic.inertie = { i, i, i };
    _physic.inertieInverse = (i > 0.0f) ? XMFLOAT3{ 1.0f / i, 1.0f / i, 1.0f / i } : XMFLOAT3{ 0.0f, 0.0f, 0.0f };
}

