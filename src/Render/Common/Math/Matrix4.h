#ifndef MATRIX4_H_DEFINED
#define MATRIX4_H_DEFINED

#include "Vector4_Fwd.h"

template <typename T>
class Matrix4
{
public:
    union 
    {
        struct
        {
            T m00, m01, m02, m03;
            T m10, m11, m12, m13;
            T m20, m21, m22, m23;
            T m30, m31, m32, m33;
        };
        
        Vector4<T> rows[4];
    };
    
    Matrix4();
    Matrix4(T _scalar);
    Matrix4(Vector4<T> _r1, Vector4<T> _r2, Vector4<T> _r3, Vector4<T> _r4);
    Matrix4(T _00, T _01, T _02, T _03,
            T _10, T _11, T _12, T _13,
            T _20, T _21, T _22, T _23,
            T _30, T _31, T _32, T _33);
    Matrix4(std::initializer_list<std::initializer_list<T>> _l);
    
    static Matrix4<T> Identity();
    static Matrix4<T> Identity(T _scalar);
    
    Matrix4 operator+(Matrix4 const& _o) const;
    Matrix4 operator-(Matrix4 const& _o) const;
    
    Matrix4& operator+=(Matrix4 const& _o);
    Matrix4& operator-=(Matrix4 const& _o);
    
    Matrix4 operator*(T _scalar)                const;
    Vector4<T> operator*(Vector3<T> const& _v)  const;
    Vector4<T> operator*(Vector4<T> const& _v)  const;
    Matrix4 operator*(Matrix4 const& _o)        const;
    
    Matrix4& operator*=(T _scalar);
    Matrix4& operator*=(Matrix4 const& _o);
    
    Matrix4     Transposed() const;
    Matrix4&    SelfTranspose();
    
    float       Determinant()           const;
    float       Minor(int _r, int _c)   const;
    Matrix4     Comatrix()              const;
    
    Matrix4     Inverted()          const;
    Matrix4     InvertedAffine()    const;
    Matrix4&    SelfInvert();
    Matrix4&    SelfInvertAffine();
    
    static float    Determinant(Matrix4 const& _m);
    static Matrix4  Transpose(Matrix4 const& _m);
    static Matrix4  Invert(Matrix4 const& _m);
    static Matrix4  InvertAffine(Matrix4 const& _m);
    
    static Matrix4  MakeTranslation(Vector4<T> const& _v);
    static Matrix4  MakeScale(Vector4<T> const& _v);
    static Matrix4  MakeRotation(Vector4<T> const& _axis, T _angle);
    static Matrix4  MakeRotationX(T _angle);
    static Matrix4  MakeRotationY(T _angle);
    static Matrix4  MakeRotationZ(T _angle);
    
    Vector4<T> const& operator[](int _i) const;
    Vector4<T>&       operator[](int _i);
    T* Data();
    T const* Data() const;
};

#include "Matrix4.inl"

#endif
