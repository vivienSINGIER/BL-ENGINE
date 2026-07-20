#ifndef SHAPE_COMPONENT_HPP_DEFINED
#define SHAPE_COMPONENT_HPP_DEFINED

#include "define.h"

enum class ShapeType : uint8
{
    Box,
    Sphere,
    Capsule
};

struct BoxShape
{
    XMFLOAT3 halfExtents = { 0.5f, 0.5f, 0.5f };
};

struct SphereShape
{
    float radius = 0.5f;
};

struct CapsuleShape
{
    float radius = 0.5f;
    float halfHeight = 1.0f;
};

struct AABBCollider
{
    XMFLOAT3 min = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 max = { 0.0f, 0.0f, 0.0f };

    bool Overlaps(const AABBCollider& _other) const
    {
        return
            min.x <= _other.max.x && max.x >= _other.min.x &&
            min.y <= _other.max.y && max.y >= _other.min.y &&
            min.z <= _other.max.z && max.z >= _other.min.z;
    }

    XMFLOAT3 Center() const
    {
        return
        {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };
    }
};

struct ColliderComponent
{
    ShapeType type = ShapeType::Box;

    union ShapeData
    {
        BoxShape     box;
        SphereShape  sphere;
        CapsuleShape capsule;

        ShapeData() : box() {}
    } shape;

    // Offset du collider par rapport à l'origine du TransformComponent.
    XMFLOAT3 localOffset = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4 localRotation = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Flags
    bool isTrigger = false;

    AABBCollider     aabb;
    XMFLOAT3 worldCenter = { 0.0f, 0.0f, 0.0f };
    float    worldRadius = 0.0f;

    XMFLOAT3 worldAxes[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    XMFLOAT3 worldHalfExtents = { 0.5f, 0.5f, 0.5f };

    void SetBox(const XMFLOAT3& _halfExtents)
    {
        type = ShapeType::Box;
        shape.box.halfExtents = _halfExtents;
    }

    void SetBox(BoundingBox& _box)
    {
        XMFLOAT3& extents = _box.Extents;
        SetBox(extents);
    }

    void SetBox(float _hx, float _hy, float _hz)
    {
        SetBox({ _hx, _hy, _hz });
    }

    void SetSphere(float _radius)
    {
        type = ShapeType::Sphere;
        shape.sphere.radius = _radius;
    }

    void SetCapsule(float _radius, float _halfHeight)
    {
        type = ShapeType::Capsule;
        shape.capsule.radius = _radius;
        shape.capsule.halfHeight = _halfHeight;
    }

    float GetLocalBoundingRadius() const
    {
        switch (type)
        {
        case ShapeType::Box:
        {
            const XMFLOAT3& h = shape.box.halfExtents;
            return sqrtf(h.x * h.x + h.y * h.y + h.z * h.z);
        }
        case ShapeType::Sphere:
            return shape.sphere.radius;

        case ShapeType::Capsule:
            return shape.capsule.halfHeight + shape.capsule.radius;
        }
        return 0.0f;
    }
};

#endif // !SHAPE_COMPONENT_HPP_DEFINED