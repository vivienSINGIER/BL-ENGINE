#ifndef AABB_H_DEFINED
#define AABB_H_DEFINED

#include "../Vector/Vector.h"
#include "../Matrix/Matrix.h"

class AABB
{
public:
    Vect3f32 min;
    Vect3f32 max;
    
    AABB();
    AABB(Vect3f32 const& _min, Vect3f32 const& _max);
    
    static AABB FromMinMax(Vect3f32 const& _min, Vect3f32 const& _max);
    static AABB FromCenterExtent(Vect3f32 const& _center, Vect3f32 const& _extent);
    
    Vect3f32 Center() const;
    Vect3f32 Extent() const;
    float SurfaceArea() const;
    
    void SetCenter(Vect3f32 const& _center);
    void SetExtent(Vect3f32 const& _extent);
    
    void Merge(AABB const& _o);
    void Union(AABB const& _o);
    void Expand(float _scalar);
    
    void Transform(Mat4f32 const& _t);
    AABB Transformed(Mat4f32 const& _t) const;
    
    // TODO Add (Ray, Sphere & OBB) intersect & contain
    
    bool Contains(Vect3f32 const& _p) const;
    bool Contains(AABB const& _o) const;
    bool Intersects(AABB const& _o) const;
    
    static AABB Merge(AABB const& _a, AABB const& _b);
    static AABB Union(AABB const& _a, AABB const& _b);
    static AABB Expand(AABB const& _o, float _scalar);
    
    static bool Contains(AABB const& _aabb, Vect3f32 const& _p);
    static bool Contains(AABB const& _aabb, AABB const& _o);
    static bool Intersects(AABB const& _a, AABB const& _b);
};

#endif
