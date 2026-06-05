#ifndef VECTOR3_H_DEFINED
#define VECTOR3_H_DEFINED

#include "Vector2.h"

template <typename T>
class Vector3
{
public:
    T x;
    T y;
    T z;
    
    Vector3();
    Vector3(T _x, T _y, T _z);
    Vector3(T _scalar);
    Vector3(Vector2<T> const& _xy);
    
    Vector3 operator+(Vector3 const& _o);
    Vector3 operator-(Vector3 const& _o);
    Vector3 operator*(Vector3 const& _o);
    Vector3 operator/(Vector3 const& _o);
    Vector3 operator^(Vector3 const& _o);
    Vector3 operator*(float _scalar);
    Vector3 operator/(float _scalar);
    
    Vector3& operator+=(Vector3 const& _o);
    Vector3& operator-=(Vector3 const& _o);
    Vector3& operator*=(Vector3 const& _o);
    Vector3& operator/=(Vector3 const& _o);
    Vector3& operator^=(Vector3 const& _o);
    Vector3& operator*=(float _scalar);
    Vector3& operator/=(float _scalar);
    
    bool operator==(Vector3 const& _o);
    bool operator!=(Vector3 const& _o);
    
    float Length();
    float LengthSquared();
    
    Vector3  Normalized();
    Vector3& SelfNormalize();
    
    Vector3 Reflect(Vector3 const& _normal);
    Vector3 Project(Vector3 const& _target);
    Vector3 Perpendicular(Vector3 const& _v);
    
    static Vector3 Zero();
    static Vector3 One();
    
    static float Dot(Vector3 const& _v1, Vector3 const& _v2);
    static Vector3 Normalize(Vector3 const& _o);
    
    static Vector3 Min(Vector3 const& _v1, Vector3 const& _v2);
    static Vector3 Max(Vector3 const& _v1, Vector3 const& _v2);
    static Vector3 Abs(Vector3 const& _v);
    static Vector3 Clamp(Vector3 const& _v, Vector3 const& _min, Vector3 const& _max);
    
    Vector2<T> xy();
    Vector2<T> yx();
    Vector2<T> xz();
    Vector2<T> zx();
    Vector2<T> yz();
    Vector2<T> zy();
    
    Vector3 xzy();
    Vector3 yxz();
    Vector3 yzx();
    Vector3 zxy();
    Vector3 zyx();
    
    T  operator[](int _i) const;
    T& operator[](int _i);
    T* Data();
};

#include "Vector2.inl"

#endif
