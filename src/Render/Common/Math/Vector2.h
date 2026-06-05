#ifndef VECTOR2_H_DEFINED
#define VECTOR2_H_DEFINED

template <typename T>
class Vector2
{
public:
    T x;
    T y;
    
    Vector2();
    Vector2(T _x, T _y);
    Vector2(T _scalar);
    
    Vector2 operator+(Vector2 const& _o);
    Vector2 operator-(Vector2 const& _o);
    Vector2 operator*(Vector2 const& _o);
    Vector2 operator/(Vector2 const& _o);
    Vector2 operator*(float _scalar);
    Vector2 operator/(float _scalar);
    
    Vector2& operator+=(Vector2 const& _o);
    Vector2& operator-=(Vector2 const& _o);
    Vector2& operator*=(Vector2 const& _o);
    Vector2& operator/=(Vector2 const& _o);
    Vector2& operator*=(float _scalar);
    Vector2& operator/=(float _scalar);
    
    bool operator==(Vector2 const& _o);
    bool operator!=(Vector2 const& _o);
    
    float DeltaAngle(Vector2 const& _o);
    
    float Length();
    float LengthSquared();
    
    Vector2  Normalized();
    Vector2& SelfNormalize();
    
    Vector2 Reflect(Vector2 const& _normal);
    Vector2 Project(Vector2 const& _target);
    Vector2 Perpendicular();
    
    static Vector2 Zero();
    static Vector2 One();
    
    static float Dot(Vector2 const& _v1, Vector2 const& _v2);
    static Vector2 Normalize(Vector2 const& _o);
    
    static Vector2 Min(Vector2 const& _v1, Vector2 const& _v2);
    static Vector2 Max(Vector2 const& _v1, Vector2 const& _v2);
    static Vector2 Abs(Vector2 const& _v);
    static Vector2 Clamp(Vector2 const& _v, Vector2 const& _min, Vector2 const& _max);
    
    Vector2 yx();
    
    T  operator[](int _i) const;
    T& operator[](int _i);
    T* Data();
};

#include "Vector2.inl"

#endif
