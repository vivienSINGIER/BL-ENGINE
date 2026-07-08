#include "Plane.h"

#include "AABB.h"
#include "OBB.h"
#include "Ray.h"
#include "Sphere.h"

Plane::Plane()
{
    normal = Vect3f32(0.0f, 1.0f, 0.0f);
    distance = 0.0f;
}

Plane::Plane(Vect3f32 const& _normal, float _dist)
{
    normal = _normal.Normalized();
    distance = _dist;
}

Plane::Plane(Vect3f32 const& _normal, Vect3f32 const& _pos)
{
    normal = _normal.Normalized();

    Vect3f32 po = - _pos;
    Vect3f32 proj = po.Project(normal);

    distance = proj.LengthSquared();
}

Plane::Plane(Vect3f32 const& _p1, Vect3f32 const& _p2, Vect3f32 const& _p3)
{
    Vect3f32 a = _p2 - _p1;
    Vect3f32 b = _p3 - _p1;

    normal = (a ^ b).Normalized();

    Vect3f32 po = - _p1;
    Vect3f32 proj = po.Project(normal);

    distance = proj.LengthSquared();
}

int Plane::ClassifyPoint(Vect3f32 const& _p) const
{
    float dot = Vect3f32::Dot(normal, _p);
    
    if (dot == 0.0f)
        return 0;
    
    return MathUtils::Sign(dot);
}

float Plane::DistanceToPoint(Vect3f32 const& _p) const
{
    return Vect3f32::Dot(normal, _p) - distance;
}

bool Plane::Contains(Vect3f32 const& _pos) const
{
    Vect3f32 o = normal * distance;
    Vect3f32 v = _pos - o;

    return Vect3f32::Dot(o, v) == 0.0f;
}

bool Plane::Intersects(Plane const& _o) const
{
    return MathUtils::Abs(Vect3f32::Dot(normal, _o.normal)) != 1.0f;
}

bool Plane::Intersects(Ray const& _r) const
{
    return _r.Intersects(*this);
}

bool Plane::Intersects(AABB const& _a) const
{
    int minPos = ClassifyPoint(_a.min);
    int maxPos = ClassifyPoint(_a.max);

    if (minPos == 0 || maxPos == 0)
        return true;

    return minPos != maxPos;
}

bool Plane::Intersects(Sphere const& _s) const
{
    float d = DistanceToPoint(_s.center);

    return d <= _s.radius;
}

bool Plane::Intersects(OBB const& _o) const
{
    float radius =  _o.extent.x * MathUtils::Abs(Vect3f32::Dot(normal, _o.orientation.rows[0])) +
                    _o.extent.y * MathUtils::Abs(Vect3f32::Dot(normal, _o.orientation.rows[1])) +
                    _o.extent.z * MathUtils::Abs(Vect3f32::Dot(normal, _o.orientation.rows[2]));

    float d =  DistanceToPoint(_o.position);
    return d <= radius;     
}




 

