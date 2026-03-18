#ifndef COLLIDER_COMPONENT_HPP_INCLUDED
#define COLLIDER_COMPONENT_HPP_INCLUDED

#include "define.h"
#include "../Core/Transform.h"

enum class ColliderType
{
	Box,
	Sphere
};

struct OBB
{
	XMFLOAT3 center;
	XMFLOAT3 axes[3];
	XMFLOAT3 halfExtents;
};

struct  ColliderComponent
{
	bool isTrigger;
	ColliderType type;
	Transform colliderTransform;
	OBB boundingBox;
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED
