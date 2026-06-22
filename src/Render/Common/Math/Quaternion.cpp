
#include "Quaternion.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include "Vector4.h"
#include "Vector3.h"

Quaternion::Quaternion() : x(0), y(0), z(0), w(1)
{
    
}

Quaternion::Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
{
}

Quaternion::Quaternion(Vector4<float> const& _v) :
    x(_v.x), y(_v.y), z(_v.z), w(_v.w)
{
    
}

Quaternion::Quaternion(std::initializer_list<float> _l)
{
    assert(_l.size() == 4 && "Incorrect init list size");
    
    const float* begin = _l.begin();
    x = begin[0];
    y = begin[1];
    z = begin[2];
    w = begin[3];
}

Quaternion Quaternion::FromEulerAngles(float _yaw, float _pitch, float _roll)
{
    Quaternion quat;
    quat.x = MathUtils::Sin(_roll / 2) * MathUtils::Cos(_pitch / 2) * MathUtils::Cos(_roll / 2) - MathUtils::Cos(_roll / 2) * MathUtils::Sin(_pitch / 2) * MathUtils::Sin(_roll / 2);
    quat.y = MathUtils::Cos(_roll / 2) * MathUtils::Sin(_pitch / 2) * MathUtils::Cos(_roll / 2) + MathUtils::Sin(_roll / 2) * MathUtils::Cos(_pitch / 2) * MathUtils::Sin(_roll / 2);
    quat.z = MathUtils::Cos(_roll / 2) * MathUtils::Cos(_pitch / 2) * MathUtils::Sin(_roll / 2) - MathUtils::Sin(_roll / 2) * MathUtils::Sin(_pitch / 2) * MathUtils::Cos(_roll / 2);
    quat.w = MathUtils::Cos(_roll / 2) * MathUtils::Cos(_pitch / 2) * MathUtils::Cos(_roll / 2) + MathUtils::Sin(_roll / 2) * MathUtils::Sin(_pitch / 2) * MathUtils::Sin(_roll / 2);
    return quat;
}

Quaternion Quaternion::FromEulerAngles(Vector3<float> const& _angles)
{
    Quaternion quat;
    quat.x = MathUtils::Sin(_angles.z / 2) * MathUtils::Cos(_angles.y / 2) * MathUtils::Cos(_angles.x / 2) - MathUtils::Cos(_angles.z / 2) * MathUtils::Sin(_angles.y / 2) * MathUtils::Sin(_angles.x / 2);
    quat.y = MathUtils::Cos(_angles.z / 2) * MathUtils::Sin(_angles.y / 2) * MathUtils::Cos(_angles.x / 2) + MathUtils::Sin(_angles.z / 2) * MathUtils::Cos(_angles.y / 2) * MathUtils::Sin(_angles.x / 2);
    quat.z = MathUtils::Cos(_angles.z / 2) * MathUtils::Cos(_angles.y / 2) * MathUtils::Sin(_angles.x / 2) - MathUtils::Sin(_angles.z / 2) * MathUtils::Sin(_angles.y / 2) * MathUtils::Cos(_angles.x / 2);
    quat.w = MathUtils::Cos(_angles.z / 2) * MathUtils::Cos(_angles.y / 2) * MathUtils::Cos(_angles.x / 2) + MathUtils::Sin(_angles.z / 2) * MathUtils::Sin(_angles.y / 2) * MathUtils::Sin(_angles.x / 2);
    return quat;
}

Quaternion Quaternion::FromAxisAngle(Vector3<float> const& _axis, float _angle)
{
    assert(_axis.Length() == 1.0f && "axis should be normalized");
    
    Quaternion quat;
    quat.x = _axis.x * MathUtils::Sin(_angle / 2);
    quat.y = _axis.y * MathUtils::Sin(_angle / 2);
    quat.z = _axis.z * MathUtils::Sin(_angle / 2);
    quat.w = MathUtils::Cos(_angle / 2);
    return quat;
}

Quaternion Quaternion::FromRotationMatrix(Matrix3<float> const& _rotMat)
{
    Matrix4<float> m = 
        {
            _rotMat.m00, _rotMat.m01, _rotMat.m02, 0.0f,
            _rotMat.m10, _rotMat.m11, _rotMat.m12, 0.0f,
            _rotMat.m20, _rotMat.m21, _rotMat.m22, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    return FromRotationMatrix(m);
}

Quaternion Quaternion::FromRotationMatrix(Matrix4<float> const& _rotMat)
{
    float x = MathUtils::Sqrt( _rotMat.m00 - _rotMat.m11 - _rotMat.m22 + 1.0f) * 0.5f;
    float y = MathUtils::Sqrt(-_rotMat.m00 + _rotMat.m11 - _rotMat.m22 + 1.0f) * 0.5f;
    float z = MathUtils::Sqrt(-_rotMat.m00 - _rotMat.m11 + _rotMat.m22 + 1.0f) * 0.5f;
    float w = MathUtils::Sqrt( _rotMat.m00 + _rotMat.m11 + _rotMat.m22 + 1.0f) * 0.5f;
    
    if (w >= x && w >= y && w >= z)
    {
        float d = 1.0f / (4.0f * w);
        x = (_rotMat.m12 - _rotMat.m21) * d;
        y = (_rotMat.m20 - _rotMat.m02) * d;
        z = (_rotMat.m01 - _rotMat.m10) * d;
    }
    else if (x >= y && x >= z)
    {
        float d = 1.0f / (4.0f * x);
        y = (_rotMat.m01 + _rotMat.m10) * d;
        z = (_rotMat.m20 + _rotMat.m02) * d;
        w = (_rotMat.m12 - _rotMat.m21) * d;
    }
    else if (y >= z)
    {
        float d = 1.0f / (4.0f * y);
        x = (_rotMat.m01 + _rotMat.m10) * d;
        z = (_rotMat.m12 + _rotMat.m21) * d;
        w = (_rotMat.m20 - _rotMat.m02) * d;
    }
    else
    {
        float d = 1.0f / (4.0f * z);
        x = (_rotMat.m20 + _rotMat.m02) * d;
        y = (_rotMat.m12 + _rotMat.m21) * d;
        w = (_rotMat.m01 - _rotMat.m10) * d;
    }
    
    return Quaternion(x, y, z, w);
}

Quaternion Quaternion::operator+(Quaternion const& _q) const
{
    return Quaternion(x + _q.x, y + _q.y, z + _q.z, w + _q.w);
}

Quaternion Quaternion::operator-(Quaternion const& _q) const
{
    return Quaternion(x - _q.x, y - _q.y, z - _q.z, w - _q.w);
}

Quaternion Quaternion::operator*(Quaternion const& _q) const
{
    float rW = w * _q.w;
    float rX = w*_q.x + x*_q.w + y*_q.z - z*_q.y;
    float rY = w*_q.y + y*_q.w + z*_q.x - x*_q.z;
    float rZ = w*_q.z + z*_q.w + x*_q.y - y*_q.x;
    return Quaternion(rX, rY, rZ, rW);
}

Quaternion& Quaternion::operator+=(Quaternion const& _q)
{
    x = x + _q.x;
    y = y + _q.y;
    z = z + _q.z;
    w = w + _q.w;
    return *this;
}

Quaternion& Quaternion::operator-=(Quaternion const& _q)
{
    x = x - _q.x;
    y = y - _q.y;
    z = z - _q.z;
    w = w - _q.w;
    return *this;
}

Quaternion& Quaternion::operator*=(Quaternion const& _q)
{
    float rW = w * _q.w;
    float rX = w*_q.x + x*_q.w + y*_q.z - z*_q.y;
    float rY = w*_q.y + y*_q.w + z*_q.x - x*_q.z;
    float rZ = w*_q.z + z*_q.w + x*_q.y - y*_q.x;
    
    x = rX;
    y = rY;
    z = rZ;
    w = rW;
    return *this;
}

float Quaternion::Dot(Quaternion const& _q) const
{
    return x * _q.x + y * _q.y + z * _q.z + w * _q.w;
}

float Quaternion::Length() const
{
    return MathUtils::Sqrt(x*x + y*y + z*z + w*w);
}

float Quaternion::LengthSquared() const
{
    return x*x + y*y + z*z + w*w;
}

Quaternion Quaternion::Conjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverted() const
{
    float d = x * x - y * y - z * z + w * w;
    
    float rX = -x / d;
    float rY = -y / d;
    float rZ = -z / d;
    float rW =  w / d;
    
    return Quaternion(rX, rY, rZ, rW);
}

Quaternion& Quaternion::SelfInvert()
{
    float d = x * x - y * y - z * z + w * w;
    
    x = -x / d;
    y = -y / d;
    z = -z / d;
    w =  w / d;
    return *this;
}

Quaternion Quaternion::Normalized() const
{
    float length = Length();
    assert(length > 0.0f && "Can't normalized null quaternion");
    
    return Quaternion(x / length, y / length, z / length, w / length);
}

Quaternion& Quaternion::SelfNormalize()
{
    float length = Length();
    assert(length > 0.0f && "Can't normalized null quaternion");
    
    x = x / length;
    y = y / length;
    z = z / length;
    w = w / length;
    return *this;
}

Quaternion Quaternion::SafeNormalized() const
{
    float length = Length();
    if (length == 0.0f || length == 1.0f) return *this;
    
    return Quaternion(x / length, y / length, z / length, w / length);
}

Quaternion& Quaternion::SafeSelfNormalize()
{
    float length = Length();
    if (length == 0.0f || length == 1.0f) return *this;
    
    x = x / length;
    y = y / length;
    z = z / length;
    w = w / length;
    return *this;
}

float Quaternion::Dot(Quaternion const& _q1, Quaternion const& _q2)
{
    return _q1.Dot(_q2);   
}

float Quaternion::Length(Quaternion const& _q)
{
    return _q.Length();
}

float Quaternion::LengthSquared(Quaternion const& _q)
{
    return _q.LengthSquared();
}

Quaternion Quaternion::Conjugate(Quaternion const& _q)
{
    return Quaternion(-_q.x, -_q.y, -_q.z, _q.w);
}

Quaternion Quaternion::Invert(Quaternion const& _q)
{
    return _q.Inverted();
}

Quaternion Quaternion::Normalize(Quaternion const& _q)
{
    return _q.Normalized();
}

Quaternion Quaternion::SafeNormalize(Quaternion const& _q)
{
    return _q.SafeNormalized();
}

bool Quaternion::operator==(Quaternion const& _q) const
{
    return x == _q.x && y == _q.y && z == _q.z && w == _q.w;
}

bool Quaternion::operator!=(Quaternion const& _q) const
{
    return x != _q.x || y != _q.y || z != _q.z || w != _q.w;
}




