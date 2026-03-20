#ifndef COLLIDER_COMPONENT_HPP_INCLUDED
#define COLLIDER_COMPONENT_HPP_INCLUDED

#include "define.h"
#include "../Core/Transform.h"

enum class ColliderType
{
	Box,
	Sphere
};

struct AABB
{
	XMFLOAT3 min;
	XMFLOAT3 max;
};

struct OBB
{
	XMFLOAT3 center;
	XMFLOAT3 axes[3];
	XMFLOAT3 halfExtents;
};

struct Contact
{
	EntityId other;
	XMFLOAT3 normal;
	XMFLOAT3 point;
	float penetration;
};

struct  ColliderComponent
{
	bool isTrigger;
	ColliderType type;
	Transform colliderTransform;
	Array<Contact, 5> contact = Array<Contact, 5>();
	int contactCount;
	OBB obb;
	AABB aabb;
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED
