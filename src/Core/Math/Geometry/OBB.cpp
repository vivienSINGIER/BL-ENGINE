#include "OBB.h"

#include "Utils.hpp"

OBB::OBB()
{
    position = Vect3f32();
    extent = Vect3f32(1.0f);
    
    orientation = Mat3f32::Identity();
}

OBB::OBB(Vect3f32 const& _pos, Vect3f32 const& _extent)
{
    position = _pos;
    extent = _extent;
    orientation = Mat3f32::Identity();
}

OBB::OBB(Vect3f32 const& _pos, Vect3f32 const& _extent, Mat3f32 const& _orientation)
{
    position = _pos;
    extent = _extent;
    orientation = _orientation;
}

void OBB::Expand(float _scalar)
{
    extent *= _scalar;
}

void OBB::Transform(Mat4f32 const& _t)
{
    Mat4f32 start = Mat4f32::MakeTransform(position, extent * 2.0f, orientation.ToMatrix4());
    
    start *= _t;
    
    start.FastDecompose(&position, &extent, &orientation);
    extent *= 0.5f;
}

OBB OBB::Transformed(Mat4f32 const& _t) const
{
    OBB result(position, extent, orientation);
    result.Transform(_t);
    
    return result;
}

bool OBB::Contains(Vect3f32 const& _pos) const
{
    Vect3f32 d = _pos - position;
    Mat3f32 rot = orientation;

    float px = Vect3f32::Dot(d, rot.rows[0]);
    if ( MathUtils::Abs(px) < extent.x )
        return false;
    
    float py = Vect3f32::Dot(d, rot.rows[1]);
    if ( MathUtils::Abs(py) < extent.y )
        return false;
    
    float pz = Vect3f32::Dot(d, rot.rows[2]);
    if ( MathUtils::Abs(pz) < extent.z )
        return false;
    
    return true;
}

bool OBB::Contains(OBB const& _o) const
{
    
}

bool OBB::IsSeparate(Vect3f32 const& _axis, OBB const& _a, OBB const& _b) const
{
    Vect3f32 center = Vect3f32::Dot(_b.position - _a.position, _axis);
    
    
}












