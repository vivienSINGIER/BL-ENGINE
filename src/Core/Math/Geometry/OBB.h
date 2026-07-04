#ifndef OBB_H_DEFINED
#define OBB_H_DEFINED

#include "../Vector/Vector.h"
#include "../Matrix/Matrix.h"

class OBB
{
public:
    Vect3f32 position;
    Vect3f32 extent;
    
    Mat3f32 orientation;
    
    OBB();
    OBB(Vect3f32 const& _pos, Vect3f32 const& _extent);
    OBB(Vect3f32 const& _pos, Vect3f32 const& _extent, Mat3f32 const& _orientation);
    
    void Expand(float _scalar);
    
    void Transform(Mat4f32 const& _t);
    OBB Transformed(Mat4f32 const& _t) const;
    
    bool Contains(Vect3f32 const& _pos) const;
    bool Contains(OBB const& _o) const;
    bool Intersects(OBB const& _o) const;
    
    static OBB Expand(OBB const& _o, float _scalar);
    
    static bool Contains(OBB const& _o, Vect3f32 const& _pos);
    static bool Intersects(OBB const& _obb, OBB const& _o);
    
private:
    bool IsSeparate(Vect3f32 const& _axis, OBB const& _a, OBB const& _b) const;
};

#endif
