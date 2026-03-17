#ifndef COLLIDER_COMPONENT_HPP_INCLUDED
#define COLLIDER_COMPONENT_HPP_INCLUDED

#include "define.h"

enum class ColliderType
{
	Box,
	Sphere
};

struct Box
{
	XMFLOAT3 halfExtents;
};

struct Sphere
{
	float radius;
};

struct ColliderComponent
{
	bool isTrigger;
	ColliderType type;
	union
	{
		Box box;
		Sphere sphere;
	};
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED
