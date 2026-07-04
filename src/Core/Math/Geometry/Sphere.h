#ifndef SPHERE_H_DEFINED
#define SPHERE_H_DEFINED

#include "../Vector/Vector.h"
#include "../Matrix/Matrix.h"

class Sphere
{
public:
    Vect3f32 center;
    float radius;
    
    Sphere();
    Sphere(Vect3f32 const& _center, float _radius);
    
    void Merge(Sphere const& _sphere);
    void Union(Sphere const& _sphere);
    void Expand(float _scalar);
    
    void   Transform(Mat4f32 const& _t);
    Sphere Transformed(Mat4f32 const& _t);
    
    // TODO Add (Ray, AABB & OBB) intersect & contain
    
    bool Contains(Vect3f32 const& _p);
    bool Contains(Sphere const& _o);
    bool Intersects(Sphere const& _o);
    
    static bool Contains(Sphere const& _sphere, Vect3f32 const& _p);
    static bool Contains(Sphere const& _sphere, Sphere const& _o);
    static bool Intersects(Sphere const& _sphere, Sphere const& _o);
    
    static Sphere Merge(Sphere const& _sphere, Sphere const& _o);
    static Sphere Union(Sphere const& _sphere, Sphere const& _o);
    static Sphere Expand(Sphere const& _sphere, float _scalar);
};

#endif
