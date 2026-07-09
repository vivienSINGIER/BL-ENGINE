#ifndef FRUSTUM_H_DEFINED
#define FRUSTUM_H_DEFINED

#include "Plane.h"
#include "Math/Matrix/Matrix.h"

class Sphere;
class AABB;
class OBB;

class Frustum
{
public:
    Frustum(bool _isGl = false);
    Frustum(Mat4f32 const& _viewProj, bool _isGl = false);

    bool Contains(Vect3f32 const& _p) const;
    bool Intesects(AABB const& _o) const;
    bool Intesects(Sphere const& _s) const;
    bool Intesects(OBB const& _o) const;
    
private:
    union 
    {
        Plane planes[6];

        struct
        {
            Plane left;
            Plane right;
            Plane bottom;
            Plane top;
            Plane nearZ;
            Plane farZ;
        };
        
    };
};

#endif
