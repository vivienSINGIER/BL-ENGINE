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
    float staticFriction = 0.1f;
    float dynamicFriction = 0.05f;

    XMFLOAT3 velocity = { 0,0,0 };
    XMFLOAT3 acceleration = { 0,0,0 };
    XMFLOAT3 forces = { 0,0,0 };
    bool useGravity = false;

    XMFLOAT3 angularVelocity = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 torque = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 inertie = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 inertieInverse = { 1.0f, 1.0f, 1.0f };
    bool rotation = false;

    bool isSleeping = false;
    float sleepTimer = 0.0f;

    bool hasSupportContact = false;
	XMFLOAT3 supportNormal = { 0.0f, 0.0f, 0.0f };

    BodyType type;

    inline void AddForce(XMFLOAT3& v)
    {
        forces = Add(forces, v);
        WakeUp();
    }

    inline void AddAcceleration(XMFLOAT3& v)
    {
        acceleration = Add(acceleration, v);
        WakeUp();
	}

    inline void AddVelocity(XMFLOAT3& v)
    {
        velocity = Add(velocity, v);
        WakeUp();
    }

    inline void AddTorque(XMFLOAT3& v)
    {
        torque = Add(torque, v);
        WakeUp();
	}

    inline void ToggleGravity()
    {
		useGravity = !useGravity;
        WakeUp();
    }

    inline void ToggleRotation()
    {
        rotation = !rotation;
		WakeUp();
	}

    inline void SetMass(float _mass)
    {
        mass = _mass;
		massInverse = (mass > 0.0f) ? 1 / mass : 0.0f;
    }

    inline void SetStatic()
    {
        type = BodyType::Static;
        Sleep();
        massInverse = 0.0f;
		inertie = XMFLOAT3(0.0f, 0.0f, 0.0f);
        inertieInverse = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    inline void SetDynamic()
    {
		type = BodyType::Dynamic;
	}

    inline void WakeUp()
    {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    inline void Sleep()
    {
        isSleeping = true;
        sleepTimer = 0.0f;
        velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        angularVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);
        torque = XMFLOAT3(0.0f, 0.0f, 0.0f);
        forces = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED