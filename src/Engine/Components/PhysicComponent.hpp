#ifndef PHYSIC_COMPONENT_HPP_DEFINED
#define PHYSIC_COMPONENT_HPP_DEFINED

#include "define.h"
#include "../Core/Utils.hpp"

enum class BodyType
{
	Static,
	Dynamic
};

struct PhysicComponent
{
    float mass = 1.0f;
	float massInverse = 1.0f;
	float restitution = 0.5f;
    float staticFriction = 0.5f;
    float dynamicFriction = 0.3f;

    XMFLOAT3 velocity = { 0,0,0 };
    XMFLOAT3 acceleration = { 0,0,0 };
    XMFLOAT3 forces = { 0,0,0 };
    bool useGravity = false;

    XMFLOAT3 angularVelocity = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 torque = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 inertie = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 inertieInverse = { 1.0f, 1.0f, 1.0f };
    float angularDamping = 0.1f;
    bool rotation = false;

    BodyType type;

    inline void AddForce(XMFLOAT3& v)
    {
        forces = Add(forces, v);
    }

    inline void AddTorque(XMFLOAT3& v)
    {
        torque = Add(torque, v);
	}

    inline void ToggleGravity()
    {
		useGravity = !useGravity;
    }

    inline void ToggleRotation()
    {
        rotation = !rotation;
	}

    inline void SetMass(float _mass)
    {
        mass = _mass;
		massInverse = (mass > 0.0f) ? 1 / mass : 0.0f;
    }

    inline void SetStatic()
    {
        type = BodyType::Static;
        massInverse = 0.0f;
		inertie = XMFLOAT3(0.0f, 0.0f, 0.0f);
        inertieInverse = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    inline void SetDynamic()
    {
		type = BodyType::Dynamic;
	}
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED