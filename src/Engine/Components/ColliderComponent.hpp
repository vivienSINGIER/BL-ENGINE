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
	XMFLOAT3 min = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 max = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

struct  ColliderComponent
{
	bool isTrigger;
	ColliderType type;
	Transform colliderTransform;
	OBB boundingBox;
};

#endif // !COLLIDER_COMPONENT_HPP_INCLUDED
