#ifndef COLLIDER_COMPONENT_HPP_INCLUDED
#define COLLIDER_COMPONENT_HPP_INCLUDED

#include "define.h"
#include "../Core/Transform.h"

enum class ColliderType
{
	Box,
	Sphere
};

struct ColliderComponent
{
	bool isTrigger;
	ColliderType type;
	Transform colliderTransform;
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED
