#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include "define.h"

struct Segment2D
{
    XMFLOAT2 pointA;
	XMFLOAT2 pointB;
};

struct Segment3D
{
    XMFLOAT3 pointA;
    XMFLOAT3 pointB;
};

struct Axis
{
    XMFLOAT2 origin;
	XMFLOAT2 direction;
};

inline float Det(XMFLOAT2 a, XMFLOAT2 b)
{
    return a.x * b.y - a.y * b.x;
}

inline float Clamp(float v)
{
    if ( v<0.0f )
        return 0.0f;
    if ( v>1.0f )
        return 1.0f;
    return v;
}

inline float Clamp(float v, float min, float max)
{
    if ( v<min )
        return min;
    if ( v>max )
        return max;
    return v;
}

inline int Clamp(int v, int min, int max)
{
    if ( v<min )
        return min;
    if ( v>max )
        return max;
    return v;
}

inline XMFLOAT3 ToColor(XMINT3 _color)
{
    XMFLOAT3 color;
    color.x = Clamp(_color.x, 0, 255)/255.0f;
    color.y = Clamp(_color.y, 0, 255)/255.0f;
    color.z = Clamp(_color.z, 0, 255)/255.0f;
    return color;
}

inline XMFLOAT3 ToColor(int _r, int _g, int _b)
{
    XMFLOAT3 color;
    color.x = Clamp(_r, 0, 255)/255.0f;
    color.y = Clamp(_g, 0, 255)/255.0f;
    color.z = Clamp(_b, 0, 255)/255.0f;
    return color;
}

#endif
