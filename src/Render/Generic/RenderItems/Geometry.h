#ifndef GEOMETRY_H_DEFINED
#define GEOMETRY_H_DEFINED

#include "../../Common/Common.h"

#include "../../Common/Utils/RenderItemHelper.hpp"
#include "../../Common/Utils/FrustumHelper.hpp"

#define DEFAULT_BOUNDING_VOLUME_TYPE VolumeType::Sphere

class Geometry
{
public:
    virtual ~Geometry() = default;
    
    virtual void SetVertexData(const Vertex* _data, uint64 _vertexCount) = 0;
    virtual void SetIndexData(const uint32* _indices, uint64 _indexCount) = 0;
    
    void SetPrimitiveTopology(PrimitiveTopology _topology) { m_primitiveTopology = _topology; }
    
    uint64 GetVertexCount() const { return m_vertexCount; }
    uint64 GetIndexCount() const { return m_indexCount; }
    
    PrimitiveTopology GetTopology() const { return m_primitiveTopology; }
    bool IsIndexed() const { return m_indexCount > 0; }

    Volume* GetVolume() { return m_boundingVolume.GetVolume(); }
    bool FrustumCheck(Frustum const& _frustum, XMFLOAT4X4 const& _world) { return GetVolume()->IsOnFrustum(_frustum, _world); }
    
protected:
    uint64 m_vertexCount = 0;
    uint64 m_indexCount = 0;
    PrimitiveTopology m_primitiveTopology = PrimitiveTopology::TriangleList;

    bool m_isDynamic = false;
    
    BoundingVolume m_boundingVolume;
    
    Geometry(bool _isDynamic, VolumeType _vType = DEFAULT_BOUNDING_VOLUME_TYPE);
};
#endif
