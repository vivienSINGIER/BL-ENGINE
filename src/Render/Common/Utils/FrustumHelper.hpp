#ifndef FRUSTUM_HELPER_HPP_DEFINED
#define FRUSTUM_HELPER_HPP_DEFINED

#include "../Common.h"
#include "RenderItemHelper.hpp"

struct Plane
{
    XMFLOAT3 normal = {0.0f, 1.0f, 0.0f};
    float distance = 0.0f;
    
    float GetSignedDistanceToPlane(XMFLOAT3 const& _point) const
    {
        return MathHelper::Dot(_point, normal) - distance;
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;
    
    Plane leftFace;
    Plane rightFace;
    
    Plane nearFace;
    Plane farFace;
};

enum class VolumeType
{
    Sphere, AABB, OBB
};

struct Volume
{
    virtual bool IsOnFrustum(Frustum const& _frustum, XMFLOAT4X4 const& _world) = 0;
    
    virtual void CalculateVolume(Vertex const* _vertices, uint64 _vertexCount) = 0;
};

struct SphereVolume : public Volume
{
    XMFLOAT3 center = {0.0f, 0.0f, 0.0f};
    float radius = 0.0f;
    
    bool IsOnForwardPlane(Plane const& _plane) const
    {
        return _plane.GetSignedDistanceToPlane(center) > -radius;
    }
    
    bool IsOnFrustum(Frustum const& _frustum, XMFLOAT4X4 const& _world) override
    {
        XMVECTOR tempScale, tempRot, tempCenter;
        XMMatrixDecompose(&tempScale, &tempRot, &tempCenter, XMLoadFloat4x4(&_world));

        XMFLOAT3 globalCenter, globalScale;
        XMStoreFloat3(&globalCenter, tempCenter);
        XMStoreFloat3(&globalScale, tempScale);

        float maxScale = MathHelper::Max(globalScale.x, MathHelper::Max(globalScale.y, globalScale.z));
        
        XMVECTOR localCenter = XMLoadFloat3(&center);
        XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, XMLoadFloat4x4(&_world));

        SphereVolume globalSphere;
        XMStoreFloat3(&globalSphere.center, worldCenter);
        globalSphere.radius = radius * maxScale;

        bool left   = globalSphere.IsOnForwardPlane(_frustum.leftFace);
        bool right  = globalSphere.IsOnForwardPlane(_frustum.rightFace);
        bool nearZ   = globalSphere.IsOnForwardPlane(_frustum.nearFace);
        bool farZ   = globalSphere.IsOnForwardPlane(_frustum.farFace);
        bool top    = globalSphere.IsOnForwardPlane(_frustum.topFace);
        bool bottom = globalSphere.IsOnForwardPlane(_frustum.bottomFace);
        //
        // OutputDebugStringA(("Left   normal: " + std::to_string(_frustum.leftFace.normal.x)   + ", " + std::to_string(_frustum.leftFace.normal.y)   + ", " + std::to_string(_frustum.leftFace.normal.z)   + " dist: " + std::to_string(_frustum.leftFace.distance)   + "\n").c_str());
        // OutputDebugStringA(("Right  normal: " + std::to_string(_frustum.rightFace.normal.x)  + ", " + std::to_string(_frustum.rightFace.normal.y)  + ", " + std::to_string(_frustum.rightFace.normal.z)  + " dist: " + std::to_string(_frustum.rightFace.distance)  + "\n").c_str());
        // OutputDebugStringA(("Top    normal: " + std::to_string(_frustum.topFace.normal.x)    + ", " + std::to_string(_frustum.topFace.normal.y)    + ", " + std::to_string(_frustum.topFace.normal.z)    + " dist: " + std::to_string(_frustum.topFace.distance)    + "\n").c_str());
        // OutputDebugStringA(("Bottom normal: " + std::to_string(_frustum.bottomFace.normal.x) + ", " + std::to_string(_frustum.bottomFace.normal.y) + ", " + std::to_string(_frustum.bottomFace.normal.z) + " dist: " + std::to_string(_frustum.bottomFace.distance) + "\n").c_str());
        // OutputDebugStringA(("Near   normal: " + std::to_string(_frustum.nearFace.normal.x)   + ", " + std::to_string(_frustum.nearFace.normal.y)   + ", " + std::to_string(_frustum.nearFace.normal.z)   + " dist: " + std::to_string(_frustum.nearFace.distance)   + "\n").c_str());
        // OutputDebugStringA(("Far    normal: " + std::to_string(_frustum.farFace.normal.x)+ ", " + std::to_string(_frustum.farFace.normal.y)    + ", " + std::to_string(_frustum.farFace.normal.z)    + " dist: " + std::to_string(_frustum.farFace.distance)    + "\n").c_str());
        //
        // OutputDebugStringA(("Sphere center: " + std::to_string(globalSphere.center.x) + ", " + std::to_string(globalSphere.center.y) + ", " + std::to_string(globalSphere.center.z) + "\n").c_str());
        // OutputDebugStringA(("Sphere radius: " + std::to_string(globalSphere.radius) + "\n").c_str());
        // OutputDebugStringA(("left: "   + std::to_string(left)   + "\n").c_str());
        // OutputDebugStringA(("right: "  + std::to_string(right)  + "\n").c_str());
        // OutputDebugStringA(("near: "   + std::to_string(nearZ)   + "\n").c_str());
        // OutputDebugStringA(("far: "    + std::to_string(farZ)    + "\n").c_str());
        // OutputDebugStringA(("top: "    + std::to_string(top)    + "\n").c_str());
        // OutputDebugStringA(("bottom: " + std::to_string(bottom) + "\n").c_str());
        //
        return (left && right && nearZ && farZ && top && bottom);
    }
    
    void CalculateVolume(Vertex const* _vertices, uint64 _vertexCount) override
    {
        if (_vertexCount == 0) return;
    
        for (int i = 0; i < _vertexCount; i++)
        {
            float d = MathHelper::Distance(_vertices[i].position, center);
            
            if (d > radius) 
                radius = d;
        }
    }
};

struct AABBVolume : public Volume
{
    bool IsOnFrustum(Frustum const& _frustum, XMFLOAT4X4 const& _world) override
    {
        
        return true;
    }
    
    void CalculateVolume(Vertex const* _vertices, uint64 _vertexCount) override
    {
        
    }
};

struct OBBVolume : public Volume
{
    virtual bool IsOnFrustum(Frustum const& _frustum, XMFLOAT4X4 const& _world)
    {
        return true;
    }
    
    void CalculateVolume(Vertex const* _vertices, uint64 _vertexCount) override
    {
        
    }
};

class BoundingVolume
{
public:
    BoundingVolume(VolumeType _type)
    {
        SetType(_type);
    }
    
    ~BoundingVolume()
    {
        DestroyVolume();
    }
    
    BoundingVolume(const BoundingVolume& _volume) = delete;
    BoundingVolume& operator=(const BoundingVolume& _volume) = delete;
    
    void SetType(VolumeType _type)
    {
        if (m_init)
            DestroyVolume();
        m_init = true;
        
        m_type = _type;
        switch (m_type)
        {
            case VolumeType::Sphere: new (m_storage) SphereVolume(); break;
            case VolumeType::AABB: new (m_storage) AABBVolume(); break;
            case VolumeType::OBB: new (m_storage) OBBVolume(); break;
        }
    }
    
    Volume* GetVolume()
    {
        return reinterpret_cast<Volume*>(m_storage);
    }
    
    const Volume* GetVolume() const
    {
        return reinterpret_cast<const Volume*>(m_storage);
    }
    
    VolumeType GetType() const
    {
        return m_type;
    }
    
private:
    void DestroyVolume()
    {
        switch (m_type)
        {
            case VolumeType::Sphere: reinterpret_cast<SphereVolume*>(m_storage)->~SphereVolume(); break;
            case VolumeType::AABB:   reinterpret_cast<AABBVolume*>  (m_storage)->~AABBVolume();   break;
            case VolumeType::OBB:    reinterpret_cast<OBBVolume*>   (m_storage)->~OBBVolume();    break;
        }
    }
    
    static const uint64 m_tempSize = sizeof(SphereVolume) > sizeof(AABBVolume) ? sizeof(AABBVolume) : sizeof(SphereVolume);
    static const uint64 m_tempAlign = alignof(SphereVolume) > alignof(AABBVolume) ? alignof(SphereVolume) : alignof(AABBVolume);
    
    static const uint64 m_maxSize = sizeof(OBBVolume) > m_tempSize ? m_tempSize : sizeof(OBBVolume);
    static const uint64 m_maxAlign = alignof(OBBVolume) > m_maxSize ? m_maxSize : alignof(OBBVolume);
    
    alignas(m_maxAlign) unsigned char m_storage[m_maxSize];
    VolumeType m_type;
    bool m_init = false;
};

#endif
