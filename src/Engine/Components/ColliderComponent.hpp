#ifndef COLLIDER_COMPONENT_HPP_DEFINED
#define COLLIDER_COMPONENT_HPP_DEFINED

#include "define.h"

enum class ColliderType
{
	Box,
	Sphere
};

struct ColliderComponent
{
	ColliderType type;
	bool isTrigger;
	XMFLOAT3 halfExtent;
	BoxCollider box;
	SphereCollider sphere;
};

struct BoxCollider
{
	XMFLOAT3 halfExtents;
};

struct SphereCollider
{
	float radius;
};

#endif // !COLLIDER_COMPONENT_HPP_DEFINED