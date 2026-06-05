#ifndef VECTOR4_H_DEFINED
#define VECTOR4_H_DEFINED

#include "Vector2.h"
#include "Vector3.h"

template <typename T>
class Vector4
{
public:
    T x;
    T y;
    T z;
    T w;
    
    Vector4();
    Vector4(T _scalar);
    Vector4(T _x, T _y, T _z, T w);
    
    Vector4(Vector2<T> const& _xy, Vector2<T> const& _zw);
    Vector4(Vector3<T> const& _xyz, T _w);
    Vector4(T _x, Vector3<T> const& _yzw);
    Vector4(Vector2<T> const& _xy, T _z, T _w);
    Vector4(T _x, Vector2<T> const& _yz, T _w);
    Vector4(T _x, T _y, Vector2<T> const& _zw);
    
    Vector4 operator+(Vector4 const& _o);
    Vector4 operator-(Vector4 const& _o);
    Vector4 operator*(Vector4 const& _o);
    Vector4 operator/(Vector4 const& _o);
    Vector4 operator*(float _scalar);
    Vector4 operator/(float _scalar);
    
    Vector4& operator+=(Vector4 const& _o);
    Vector4& operator-=(Vector4 const& _o);
    Vector4& operator*=(Vector4 const& _o);
    Vector4& operator/=(Vector4 const& _o);
    Vector4& operator*=(float _scalar);
    Vector4& operator/=(float _scalar);
    
    bool operator==(Vector4 const& _o);
    bool operator!=(Vector4 const& _o);
    
    bool IsNull();
    
    float Length();
    float LengthSquared();
    
    Vector4  Normalized();
    Vector4& SelfNormalize();
    
    static Vector4 Zero();
    static Vector4 One();
    
    static float Dot(Vector4 const& _v1, Vector4 const& _v2);
    static Vector4 Normalize(Vector4 const& _o);
    
    static Vector4 Min(Vector4 const& _v1, Vector4 const& _v2);
    static Vector4 Max(Vector4 const& _v1, Vector4 const& _v2);
    static Vector4 Abs(Vector4 const& _v);
    static Vector4 Clamp(Vector4 const& _v, Vector4 const& _min, Vector4 const& _max);
    
    Vector2<T> xy(); Vector2<T> yx();
    Vector2<T> xz(); Vector2<T> zx();
    Vector2<T> xw(); Vector2<T> wx();
    Vector2<T> yz(); Vector2<T> zy();
    Vector2<T> yw(); Vector2<T> wy();
    Vector2<T> zw(); Vector2<T> wz();
    
    Vector3<T> xyz(); Vector3<T> xyw();
    Vector3<T> xzy(); Vector3<T> xzw();
    Vector3<T> xwy(); Vector3<T> xwz();

    Vector3<T> yxz(); Vector3<T> yxw();
    Vector3<T> yzx(); Vector3<T> yzw();
    Vector3<T> ywx(); Vector3<T> ywz();

    Vector3<T> zxy(); Vector3<T> zxw();
    Vector3<T> zyx(); Vector3<T> zyw();
    Vector3<T> zwx(); Vector3<T> zwy();

    Vector3<T> wxy(); Vector3<T> wxz();
    Vector3<T> wyx(); Vector3<T> wyz();
    Vector3<T> wzx(); Vector3<T> wzy();
    
    Vector4<T> xywz();
    Vector4<T> xzyw(); Vector4<T> xzwy();
    Vector4<T> xwyz(); Vector4<T> xwzy();

    Vector4<T> yxzw(); Vector4<T> yxwz();
    Vector4<T> yzxw(); Vector4<T> yzwx();
    Vector4<T> ywxz(); Vector4<T> ywzx();

    Vector4<T> zxyw(); Vector4<T> zxwy();
    Vector4<T> zyxw(); Vector4<T> zywx();
    Vector4<T> zwxy(); Vector4<T> zwyx();

    Vector4<T> wxyz(); Vector4<T> wxzy();
    Vector4<T> wyxz(); Vector4<T> wyzx();
    Vector4<T> wzxy(); Vector4<T> wzyx();
    
    T  operator[](int _i) const;
    T& operator[](int _i);
    T* Data();
};

#include "Vector4.inl"

#endif
