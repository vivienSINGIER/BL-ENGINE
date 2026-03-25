#include "PhysicIntegrateSystem.h"

void PhysicIntegrateSystem::OnStartUpdate(float _dt)
{
}

void PhysicIntegrateSystem::OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform)
{
    if (_physic.type == BodyType::Static)
        return;

    XMFLOAT3 dragForce = Mul(_physic.velocity, - m_airDrag);
    XMFLOAT3 totalForces = Add(_physic.forces, dragForce);

    XMFLOAT3 acceleration = Mul(totalForces, _physic.massInverse);

    if (_physic.useGravity)
        acceleration = Add(acceleration, m_gravityAccel);

    _physic.acceleration = acceleration;
    _physic.velocity = Add(_physic.velocity, Mul(acceleration, _dt));

    XMFLOAT3 move = Mul(_physic.velocity, _dt);
    _transform.local.Move(move);

    _physic.forces = XMFLOAT3(0.0f, 0.0f, 0.0f);

}

void PhysicIntegrateSystem::OnEndUpdate(float _dt)
{
}