#ifndef RENDERITEM_UTILS_HPP
#define RENDERITEM_UTILS_HPP

#include "../../Common/Common.h"

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

enum class PrimitiveTopology
{
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip
};

#endif
