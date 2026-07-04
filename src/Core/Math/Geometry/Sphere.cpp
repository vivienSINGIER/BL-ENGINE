#include "Sphere.h"

#include "Utils.hpp"

Sphere::Sphere()
{
    center = Vect3f32(0.0f);
    radius = 1.0f;
}

Sphere::Sphere(Vect3f32 const& _center, float _radius)
{
    center = _center;
    radius = _radius;
}

void Sphere::Merge(Sphere const& _sphere)
{
    center = (center + _sphere.center) / 2.0f;
    radius = (radius + _sphere.radius) / 2.0f;
}

void Sphere::Union(Sphere const& _sphere)
{
    Vect3f32 newCenter = (center + _sphere.center) / 2.0f;
    float disToCenter = (newCenter - center).Length();
    
    center = newCenter;
    radius = disToCenter + MathUtils::Max(radius, _sphere.radius);
}

void Sphere::Expand(float _scalar)
{
    radius *= _scalar;
}

void Sphere::Transform(Mat4f32 const& _t)
{
    Vect3f32 position;
    Vect3f32 scale;
    
    Mat4f32::FastDecompose(_t, &position, &scale, nullptr);
    
    center = position;
    radius *= (scale.x + scale.y + scale.z) / 3.0f; 
}

Sphere Sphere::Transformed(Mat4f32 const& _t)
{
    Vect3f32 position;
    Vect3f32 scale;
    
    Mat4f32::FastDecompose(_t, &position, &scale, nullptr);
    
    Vect3f32 nCenter = position;
    float nRadius = radius * (scale.x + scale.y + scale.z) / 3.0f; 
    return Sphere(nCenter, nRadius);
}

// TODO use Squared length when possible : avoid expensive Sqrt calculations

bool Sphere::Contains(Vect3f32 const& _p)
{
    float dist = (center - _p).Length();
    
    return dist <= radius;
}

bool Sphere::Contains(Sphere const& _o)
{
    float dist = (center - _o.center).Length();
    
    return dist + _o.radius <= radius;
}

bool Sphere::Intersects(Sphere const& _o)
{
    float dist = (center - _o.center).Length();

    return dist <= radius + _o.radius;
}

bool Sphere::Contains(Sphere const& _sphere, Vect3f32 const& _p)
{
    float dist = (_sphere.center - _p).Length();
    
    return dist <= _sphere.radius;
}

bool Sphere::Contains(Sphere const& _sphere, Sphere const& _o)
{
    float dist = (_sphere.center - _o.center).Length();
    
    return dist + _o.radius <= _sphere.radius;
}

bool Sphere::Intersects(Sphere const& _sphere, Sphere const& _o)
{
    float dist = (_sphere.center - _o.center).Length();

    return dist <= _sphere.radius + _o.radius;
}

Sphere Sphere::Merge(Sphere const& _sphere, Sphere const& _o)
{
    Vect3f32 nCenter = (_sphere.center + _o.center) / 2.0f;
    float nRadius = (_sphere.radius + _o.radius) / 2.0f;
    
    return Sphere(nCenter, nRadius);
}

Sphere Sphere::Union(Sphere const& _sphere, Sphere const& _o)
{
    Vect3f32 newCenter = (_sphere.center + _o.center) / 2.0f;
    float disToCenter = (newCenter - _sphere.center).Length();
    
    Vect3f32 nCenter = newCenter;
    float nRadius = disToCenter + MathUtils::Max(_o.radius, _sphere.radius);
    
    return Sphere(nCenter, nRadius);
}

Sphere Sphere::Expand(Sphere const& _sphere, float _scalar)
{
    return Sphere(_sphere.center, _sphere.radius * _scalar);
}

















