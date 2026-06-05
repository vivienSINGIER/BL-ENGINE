#ifndef VECTOR4_INL_DEFINED
#define VECTOR4_INL_DEFINED

#include "Vector4.h"

template <typename T>
Vector4<T>::Vector4()
{
    x = T();
    y = T();
    z = T();
    w = T();
}

template <typename T>
Vector4<T>::Vector4(T _scalar)
{
    x = _scalar;
    y = _scalar;
    z = _scalar;
    w = _scalar;
}

template <typename T>
Vector4<T>::Vector4(T _x, T _y, T _z, T _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

template <typename T>
Vector4<T>::Vector4(Vector2<T> const& _xy, Vector2<T> const& _zw)
{
    x = _xy.x;
    y = _xy.y;
    z = _zw.x;
    w = _zw.y;
}

template <typename T>
Vector4<T>::Vector4(Vector3<T> const& _xyz, T _w)
{
    x = _xyz.x;
    y = _xyz.y;
    z = _xyz.z;
    w = _w;
}

template <typename T>
Vector4<T>::Vector4(T _x, Vector3<T> const& _yzw)
{
    x = _x;
    y = _yzw.x;
    z = _yzw.y;
    w = _yzw.z;
}

template <typename T>
Vector4<T>::Vector4(Vector2<T> const& _xy, T _z, T _w)
{
    x = _xy.x;
    y = _xy.y;
    z = _z;
    w = _w;
}

template <typename T>
Vector4<T>::Vector4(T _x, Vector2<T> const& _yz, T _w)
{
    x = _x;
    y = _yz.x;
    z = _yz.y;
    w = _w;
}

template <typename T>
Vector4<T>::Vector4(T _x, T _y, Vector2<T> const& _zw)
{
    x = _x;
    y = _y;
    z = _zw.x;
    w = _zw.y;
}

template <typename T>
Vector4<T> Vector4<T>::operator+(Vector4 const& _o)
{
    return Vector4(x + _o.x, y + _o.y, z + _o.z, w + _o.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator-(Vector4 const& _o)
{
    return Vector4(x - _o.x, y - _o.y, z - _o.z, w - _o.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator*(Vector4 const& _o)
{
    return Vector4(x * _o.x, y * _o.y, z * _o.z, w * _o.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator/(Vector4 const& _o)
{
    assert(_o.x != 0.0f && _o.y != 0.0f && _o.z != 0.0f && _o.w != 0.0f && "Division by zero");
    return Vector4(x / _o.x, y / _o.y, z / _o.z, w / _o.w);
}

template <typename T>
Vector4<T> Vector4<T>::operator*(float _scalar)
{
    return Vector4(x * _scalar, y * _scalar, z * _scalar, w * _scalar);
}

template <typename T>
Vector4<T> Vector4<T>::operator/(float _scalar)
{
    assert(_scalar != 0.0f && "Division by zero");
    return Vector4(x / _scalar, y / _scalar, z / _scalar, w / _scalar);
}

template <typename T>
Vector4<T>& Vector4<T>::operator+=(Vector4 const& _o)
{
    x += _o.x;
    y += _o.y;
    z += _o.z;
    w += _o.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator-=(Vector4 const& _o)
{
    x -= _o.x;
    y -= _o.y;
    z -= _o.z;
    w -= _o.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator*=(Vector4 const& _o)
{
    x *= _o.x;
    y *= _o.y;
    z *= _o.z;
    w *= _o.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator/=(Vector4 const& _o)
{
    assert(_o.x != 0.0f && _o.y != 0.0f && _o.z != 0.0f && _o.w != 0.0f && "Division by zero");
    x /= _o.x;
    y /= _o.y;
    z /= _o.z;
    w /= _o.w;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator*=(float _scalar)
{
    x *= _scalar;
    y *= _scalar;
    z *= _scalar;
    w *= _scalar;
    return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator/=(float _scalar)
{
    assert(_scalar != 0.0f && "Division by zero");
    x /= _scalar;
    y /= _scalar;
    z /= _scalar;
    w /= _scalar;
    return *this;
}

template <typename T>
bool Vector4<T>::operator==(Vector4 const& _o)
{
    return x == _o.x && y == _o.y && z == _o.z && w == _o.w;
}

template <typename T>
bool Vector4<T>::operator!=(Vector4 const& _o)
{
    return x != _o.x || y != _o.y || z != _o.z || w != _o.w;
}

template <typename T>
bool Vector4<T>::IsNull()
{
    return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
}

template <typename T>
float Vector4<T>::Length()
{
    return MathUtils::Sqrt(x * x + y * y + z * z + w * w);
}

template <typename T>
float Vector4<T>::LengthSquared()
{
    return x * x + y * y + z * z + w * w;
}

template <typename T>
Vector4<T> Vector4<T>::Normalized()
{
    float length = Length();
    if (length == 0.0f || length == 1.0f)
        return *this;
    return Vector4<T>(x / length, y / length, z / length, w / length);
}

template <typename T>
Vector4<T>& Vector4<T>::SelfNormalize()
{
    float length = Length();
    if (length == 0.0f || length == 1.0f)
        return *this;
    
    x /= length;
    y /= length;
    z /= length;
    w /= length;
    return *this;
}

template <typename T>
Vector4<T> Vector4<T>::Zero()
{
    return Vector4<T>(0, 0, 0, 0);
}

template <typename T>
Vector4<T> Vector4<T>::One()
{
    return Vector4<T>(1, 1, 1, 1);
}

template <typename T>
float Vector4<T>::Dot(Vector4 const& _v1, Vector4 const& _v2)
{
    return _v1.x * _v2.x + _v1.y * _v2.y + _v1.z * _v2.z + _v1.w * _v2.w;
}

template <typename T>
Vector4<T> Vector4<T>::Normalize(Vector4 const& _o)
{
    return _o.Normalized();
}

template <typename T>
Vector4<T> Vector4<T>::Min(Vector4 const& _v1, Vector4 const& _v2)
{
    return Vector4<T>(MathUtils::Min(_v1.x, _v2.x), MathUtils::Min(_v1.y, _v2.y), MathUtils::Min(_v1.z, _v2.z), MathUtils::Min(_v1.w, _v2.w));
}

template <typename T>
Vector4<T> Vector4<T>::Max(Vector4 const& _v1, Vector4 const& _v2)
{
    return Vector4<T>(MathUtils::Max(_v1.x, _v2.x), MathUtils::Max(_v1.y, _v2.y), MathUtils::Max(_v1.z, _v2.z), MathUtils::Max(_v1.w, _v2.w));
}

template <typename T>
Vector4<T> Vector4<T>::Abs(Vector4 const& _v)
{
    return Vector4<T>(MathUtils::Abs(_v.x), MathUtils::Abs(_v.y), MathUtils::Abs(_v.z), MathUtils::Abs(_v.w));
}

template <typename T>
Vector4<T> Vector4<T>::Clamp(Vector4 const& _v, Vector4 const& _min, Vector4 const& _max)
{
    return Vector4<T>(MathUtils::Clamp(_v.x, _min.x, _max.x), MathUtils::Clamp(_v.y, _min.y, _max.y), MathUtils::Clamp(_v.z, _min.z, _max.z), MathUtils::Clamp(_v.w, _min.w, _max.w));
}


template <typename T>
T Vector4<T>::operator[](int _i) const
{
    return &x[_i];
}

template <typename T>
T& Vector4<T>::operator[](int _i)
{
    return &x[_i];
}

template <typename T>
T* Vector4<T>::Data()
{
    return &x;
}


#pragma region 2ComponentGetters

template <typename T>
Vector2<T> Vector4<T>::xy()
{
    return Vector2<T>(x, y);
}

template <typename T>
Vector2<T> Vector4<T>::yx()
{
    return Vector2<T>(y, x);
}

template <typename T>
Vector2<T> Vector4<T>::xz()
{
    return Vector2<T>(x, z);
}

template <typename T>
Vector2<T> Vector4<T>::zx()
{
    return Vector2<T>(z, x);
}

template <typename T>
Vector2<T> Vector4<T>::xw()
{
    return Vector2<T>(x, w);
}

template <typename T>
Vector2<T> Vector4<T>::wx()
{
    return Vector2<T>(w, x);
}

template <typename T>
Vector2<T> Vector4<T>::yz()
{
    return Vector2<T>(y, z);
}

template <typename T>
Vector2<T> Vector4<T>::zy()
{
    return Vector2<T>(z, y);
}

template <typename T>
Vector2<T> Vector4<T>::yw()
{
    return Vector2<T>(y, w);
}

template <typename T>
Vector2<T> Vector4<T>::wy()
{
    return Vector2<T>(w, y);
}

template <typename T>
Vector2<T> Vector4<T>::zw()
{
    return Vector2<T>(z, w);
}

template <typename T>
Vector2<T> Vector4<T>::wz()
{
    return Vector2<T>(w, z);
}

#pragma endregion

#pragma region 3ComponentGetters

template <typename T>
Vector3<T> Vector4<T>::xyz() { return Vector3<T>(x, y, z); }

template <typename T>
Vector3<T> Vector4<T>::xyw() { return Vector3<T>(x, y, w); }

template <typename T>
Vector3<T> Vector4<T>::xzy() { return Vector3<T>(x, z, y); }

template <typename T>
Vector3<T> Vector4<T>::xzw() { return Vector3<T>(x, z, w); }

template <typename T>
Vector3<T> Vector4<T>::xwy() { return Vector3<T>(x, w, y); }

template <typename T>
Vector3<T> Vector4<T>::xwz() { return Vector3<T>(x, w, z); }

template <typename T>
Vector3<T> Vector4<T>::yxz() { return Vector3<T>(y, x, z); }

template <typename T>
Vector3<T> Vector4<T>::yxw() { return Vector3<T>(y, x, w); }

template <typename T>
Vector3<T> Vector4<T>::yzx() { return Vector3<T>(y, z, x); }

template <typename T>
Vector3<T> Vector4<T>::yzw() { return Vector3<T>(y, z, w); }

template <typename T>
Vector3<T> Vector4<T>::ywx() { return Vector3<T>(y, w, x); }

template <typename T>
Vector3<T> Vector4<T>::ywz() { return Vector3<T>(y, w, z); }

template <typename T>
Vector3<T> Vector4<T>::zxy() { return Vector3<T>(z, x, y); }

template <typename T>
Vector3<T> Vector4<T>::zxw() { return Vector3<T>(z, x, w); }

template <typename T>
Vector3<T> Vector4<T>::zyx() { return Vector3<T>(z, y, x); }

template <typename T>
Vector3<T> Vector4<T>::zyw() { return Vector3<T>(z, y, w); }

template <typename T>
Vector3<T> Vector4<T>::zwx() { return Vector3<T>(z, w, x); }

template <typename T>
Vector3<T> Vector4<T>::zwy() { return Vector3<T>(z, w, y); }

template <typename T>
Vector3<T> Vector4<T>::wxy() { return Vector3<T>(w, x, y); }

template <typename T>
Vector3<T> Vector4<T>::wxz() { return Vector3<T>(w, x, z); }

template <typename T>
Vector3<T> Vector4<T>::wyx() { return Vector3<T>(w, y, x); }

template <typename T>
Vector3<T> Vector4<T>::wyz() { return Vector3<T>(w, y, z); }

template <typename T>
Vector3<T> Vector4<T>::wzx() { return Vector3<T>(w, z, x); }

template <typename T>
Vector3<T> Vector4<T>::wzy() { return Vector3<T>(w, z, y); }

#pragma endregion

#pragma region 4ComponentGetters
template <typename T>
Vector4<T> Vector4<T>::xywz() { return Vector4<T>(x, y, w, z); }

template <typename T>
Vector4<T> Vector4<T>::xzyw() { return Vector4<T>(x, z, y, w); }

template <typename T>
Vector4<T> Vector4<T>::xzwy() { return Vector4<T>(x, z, w, y); }

template <typename T>
Vector4<T> Vector4<T>::xwyz() { return Vector4<T>(x, w, y, z); }

template <typename T>
Vector4<T> Vector4<T>::xwzy() { return Vector4<T>(x, w, z, y); }

template <typename T>
Vector4<T> Vector4<T>::yxzw() { return Vector4<T>(y, x, z, w); }

template <typename T>
Vector4<T> Vector4<T>::yxwz() { return Vector4<T>(y, x, w, z); }

template <typename T>
Vector4<T> Vector4<T>::yzxw() { return Vector4<T>(y, z, x, w); }

template <typename T>
Vector4<T> Vector4<T>::yzwx() { return Vector4<T>(y, z, w, x); }

template <typename T>
Vector4<T> Vector4<T>::ywxz() { return Vector4<T>(y, w, x, z); }

template <typename T>
Vector4<T> Vector4<T>::ywzx() { return Vector4<T>(y, w, z, x); }

template <typename T>
Vector4<T> Vector4<T>::zxyw() { return Vector4<T>(z, x, y, w); }

template <typename T>
Vector4<T> Vector4<T>::zxwy() { return Vector4<T>(z, x, w, y); }

template <typename T>
Vector4<T> Vector4<T>::zyxw() { return Vector4<T>(z, y, x, w); }

template <typename T>
Vector4<T> Vector4<T>::zywx() { return Vector4<T>(z, y, w, x); }

template <typename T>
Vector4<T> Vector4<T>::zwxy() { return Vector4<T>(z, w, x, y); }

template <typename T>
Vector4<T> Vector4<T>::zwyx() { return Vector4<T>(z, w, y, x); }

template <typename T>
Vector4<T> Vector4<T>::wxyz() { return Vector4<T>(w, x, y, z); }

template <typename T>
Vector4<T> Vector4<T>::wxzy() { return Vector4<T>(w, x, z, y); }

template <typename T>
Vector4<T> Vector4<T>::wyxz() { return Vector4<T>(w, y, x, z); }

template <typename T>
Vector4<T> Vector4<T>::wyzx() { return Vector4<T>(w, y, z, x); }

template <typename T>
Vector4<T> Vector4<T>::wzxy() { return Vector4<T>(w, z, x, y); }

template <typename T>
Vector4<T> Vector4<T>::wzyx() { return Vector4<T>(w, z, y, x); }

#pragma endregion

#endif
