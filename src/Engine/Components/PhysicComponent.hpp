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

    XMFLOAT3 velocity = { 0,0,0 };
    XMFLOAT3 acceleration = { 0,0,0 };

    XMFLOAT3 forces = { 0,0,0 };

    BodyType type;
    bool useGravity = false;

    inline void AddForce(XMFLOAT3& v)
    {
        Add(forces, v);
    }

    inline void ToggleGravity()
    {
        if (useGravity)
            useGravity = false;
        else
            useGravity = true;
    }
};

#endif // !PHYSIC_COMPONENT_HPP_DEFINED