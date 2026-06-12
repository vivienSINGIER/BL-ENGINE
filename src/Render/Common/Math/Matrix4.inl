#ifndef MATRIX4_INL_DEFINED
#define MATRIX4_INL_DEFINED

#include "Matrix4.h"

template <typename T>
Matrix4<T>::Matrix4() : m00(), m01(), m02(), m03(), 
                        m10(), m11(), m12(), m13(), 
                        m20(), m21(), m22(), m23(), 
                        m30(), m31(), m32(), m33() 
{
}

template <typename T>
Matrix4<T>::Matrix4(T _scalar)
{
    rows[0] = Vector4<T>(_scalar, _scalar, _scalar, _scalar);
    rows[1] = Vector4<T>(_scalar, _scalar, _scalar, _scalar);
    rows[2] = Vector4<T>(_scalar, _scalar, _scalar, _scalar);
    rows[3] = Vector4<T>(_scalar, _scalar, _scalar, _scalar);
}

template <typename T>
Matrix4<T>::Matrix4(Vector4<T> _r1, Vector4<T> _r2, Vector4<T> _r3, Vector4<T> _r4)
{
    rows[0] = _r1;
    rows[1] = _r2;
    rows[2] = _r3;
    rows[3] = _r4;
}

template <typename T>
Matrix4<T>::Matrix4(
    T _11, T _12, T _13, T _14, 
    T _21, T _22, T _23, T _24, 
    T _31, T _32, T _33, T _34, 
    T _41, T _42, T _43, T _44)
{
    rows[0] = Vector4<T>(_11, _12, _13, _14);
    rows[1] = Vector4<T>(_21, _22, _23, _24);
    rows[2] = Vector4<T>(_31, _32, _33, _34);
    rows[3] = Vector4<T>(_41, _42, _43, _44);
}

template <typename T>
Matrix4<T>::Matrix4(std::initializer_list<std::initializer_list<T>> _l)
{
    assert(_l.size() == 4 && "Incorrect init list size");
    for (int i = 0; i < 4; i++)
    {
        assert(_l[i].size() == 4 && "Incorrect init list size");
        for (int j = 0; j < 4; j++)
        {
            rows[i][j] = _l[i][j];
        }
    }
}

template <typename T>
Matrix4<T> Matrix4<T>::Identity()
{
    Matrix4<T> result;
    result.rows[0] = Vector4<T>(1, 0, 0, 0);
    result.rows[1] = Vector4<T>(0, 1, 0, 0);
    result.rows[2] = Vector4<T>(0, 0, 1, 0);
    result.rows[3] = Vector4<T>(0, 0, 0, 1);
    return result;
}

template <typename T>
Matrix4<T> Matrix4<T>::Identity(T _scalar)
{
    Matrix4<T> result;
    result.rows[0] = Vector4<T>(_scalar, 0, 0, 0);
    result.rows[1] = Vector4<T>(0, _scalar, 0, 0);
    result.rows[2] = Vector4<T>(0, 0, _scalar, 0);
    result.rows[3] = Vector4<T>(0, 0, 0, _scalar);
    return result;
}

template <typename T>
Matrix4<T> Matrix4<T>::operator+(Matrix4 const& _o) const
{
    return Matrix4<T>(
        rows[0] + _o.rows[0],
        rows[1] + _o.rows[1],
        rows[2] + _o.rows[2],
        rows[3] + _o.rows[3]
        );
}

template <typename T>
Matrix4<T> Matrix4<T>::operator-(Matrix4 const& _o) const
{
    return Matrix4<T>(
        rows[0] - _o.rows[0],
        rows[1] - _o.rows[1],
        rows[2] - _o.rows[2],
        rows[3] - _o.rows[3]
        );
}

template <typename T>
Matrix4<T>& Matrix4<T>::operator+=(Matrix4 const& _o)
{
    rows[0] += _o.rows[0];
    rows[1] += _o.rows[1];
    rows[2] += _o.rows[2];
    rows[3] += _o.rows[3];
    return *this;
}

template <typename T>
Matrix4<T>& Matrix4<T>::operator-=(Matrix4 const& _o)
{
    rows[0] -= _o.rows[0];
    rows[1] -= _o.rows[1];
    rows[2] -= _o.rows[2];
    rows[3] -= _o.rows[3];
    return *this;
}

template <typename T>
Matrix4<T> Matrix4<T>::operator*(T _scalar) const
{
    return Matrix4<T>(
        rows[0] * _scalar,
        rows[1] * _scalar,
        rows[2] * _scalar,
        rows[3] * _scalar);
}

template <typename T>
Vector4<T> Matrix4<T>::operator*(Vector3<T> const& _v) const
{
    Vector4<T> result;
    result.x = m00 * _v.x + m01 * _v.y + m02 * _v.z + m03;
    result.y = m10 * _v.x + m11 * _v.y + m12 * _v.z + m13;
    result.z = m20 * _v.x + m21 * _v.y + m22 * _v.z + m23;
    result.w = m30 * _v.x + m31 * _v.y + m32 * _v.z + m33;
    return result;
}

template <typename T>
Vector4<T> Matrix4<T>::operator*(Vector4<T> const& _v) const
{
    Vector4<T> result;
    result.x = m00 * _v.x + m01 * _v.y + m02 * _v.z + m03 * _v.w;
    result.y = m10 * _v.x + m11 * _v.y + m12 * _v.z + m13 * _v.w;
    result.z = m20 * _v.x + m21 * _v.y + m22 * _v.z + m23 * _v.w;
    result.w = m30 * _v.x + m31 * _v.y + m32 * _v.z + m33 * _v.w;
    return result;
}

template <typename T>
Matrix4<T> Matrix4<T>::operator*(Matrix4 const& _o) const
{
    Matrix4<T> result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result[i][j] =
                *this[i][0] * _o[0][j] +
                *this[i][1] * _o[1][j] +
                *this[i][2] * _o[2][j] +
                *this[i][3] * _o[3][j];
        }
    }
    return result;
}

template <typename T>
Matrix4<T>& Matrix4<T>::operator*=(T _scalar)
{
    rows[0] *= _scalar;
    rows[1] *= _scalar;
    rows[2] *= _scalar;
    rows[3] *= _scalar;
    return *this;
}

template <typename T>
Matrix4<T>& Matrix4<T>::operator*=(Matrix4 const& _o)
{
    Matrix4<T> result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result[i][j] =
                *this[i][0] * _o[0][j] +
                *this[i][1] * _o[1][j] +
                *this[i][2] * _o[2][j] +
                *this[i][3] * _o[3][j];
        }
    }
    rows[0] = result.rows[0];
    rows[1] = result.rows[1];
    rows[2] = result.rows[2];
    rows[3] = result.rows[3];
    
    return *this;
}

template <typename T>
Matrix4<T> Matrix4<T>::Transposed() const
{
    Matrix4<T> result;
    result.rows[0] = Vector4<T>(m00, m10, m20, m30);
    result.rows[1] = Vector4<T>(m01, m11, m21, m31);
    result.rows[2] = Vector4<T>(m02, m12, m22, m32);
    result.rows[3] = Vector4<T>(m03, m13, m23, m33);
    return result;
}

template <typename T>
Matrix4<T>& Matrix4<T>::SelfTranspose()
{
    Matrix4<T> result;
    result.rows[0] = Vector4<T>(m00, m10, m20, m30);
    result.rows[1] = Vector4<T>(m01, m11, m21, m31);
    result.rows[2] = Vector4<T>(m02, m12, m22, m32);
    result.rows[3] = Vector4<T>(m03, m13, m23, m33);
    
    rows[0] = result.rows[0];
    rows[1] = result.rows[1];
    rows[2] = result.rows[2];
    rows[3] = result.rows[3];
    return *this;
}

template <typename T>
float Matrix4<T>::Determinant() const
{
    // Hardcoded Laplace Formula
    
    auto Det = [](float _a, float _b, float _c, float _d) -> float
    {
        return _a * _d - _b * _c;
    };
    
    float s0 = Det(m00, m01, m10, m11);
    float s1 = Det(m00, m02, m10, m12);
    float s2 = Det(m00, m03, m10, m13);
    float s3 = Det(m01, m02, m11, m12);
    float s4 = Det(m01, m03, m11, m13);
    float s5 = Det(m02, m03, m12, m13);
    
    float c0 = Det(m20, m21, m30, m31);
    float c1 = Det(m20, m22, m30, m32);
    float c2 = Det(m20, m23, m30, m33);
    float c3 = Det(m21, m22, m31, m32);
    float c4 = Det(m21, m23, m31, m33);
    float c5 = Det(m22, m23, m32, m33);
    
    return s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0;
}

template <typename T>
float Matrix4<T>::Minor(int _r, int _c) const
{
    T sub[3][3];
    int sRowIndex = 0;
    
    for (int i = 0; i < 4; i++)
    {
        if (i == _r) continue;
        int sColIndex = 0;
        for (int j = 0; j < 4; j++)
        {
            if (j == _c) continue;
            sub[sRowIndex][sColIndex] = rows[i][j];
            sColIndex++;
        }
        sRowIndex++;
    }
    
    return sub[0][0] * (sub[1][1]*sub[2][2] - sub[1][2]*sub[2][1])
         - sub[0][1] * (sub[1][0]*sub[2][2] - sub[1][2]*sub[2][0])
         + sub[0][2] * (sub[1][0]*sub[2][1] - sub[1][1]*sub[2][0]); 
}

template <typename T>
Matrix4<T> Matrix4<T>::Comatrix() const
{
    Matrix4 result;
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            T sign = ((r + c) % 2 == 0) ? T(1) : T(-1);
            result[r][c] = sign * Minor(r, c);
        }   
    }
    return result;
}

template <typename T>
Matrix4<T> Matrix4<T>::Inverted() const
{
    float det = Determinant();
    assert(det != 0 && "Matrix isn't invertible");
    
    float invDet = 1.0f / det;
    
    Matrix4<T> result = Comatrix() * invDet;
    return result;
}

template <typename T>
Matrix4<T> Matrix4<T>::InvertedAffine() const
{
    Matrix3<T> rotScal = Matrix3<T>(rows[0].xyz(), rows[1].xyz(), rows[2].xyz());
    Matrix3<T> inv = rotScal.Inverted();
    
    Vector3<T> translation = rows[3].xyz();
    Vector3<T> invTrans = translation * inv;
    invTrans = -invTrans;
    
    Matrix4<T> result;
    result.rows[0] = Vector4<T>(inv.rows[0].xyz, T(0));
    result.rows[1] = Vector4<T>(inv.rows[1].xyz, T(0));
    result.rows[2] = Vector4<T>(inv.rows[2].xyz, T(0));
    result.rows[3] = Vector4<T>(invTrans,        T(1));
    return result;
}

template <typename T>
Matrix4<T>& Matrix4<T>::SelfInvert()
{
    float det = Determinant();
    assert(det != 0 && "Matrix isn't invertible");
    
    float invDet = 1.0f / det;
    
    Matrix4<T> result = Comatrix() * invDet;
    rows[0] = result.rows[0];
    rows[1] = result.rows[1];
    rows[2] = result.rows[2];
    rows[3] = result.rows[3];
    return *this;
}

template <typename T>
Matrix4<T>& Matrix4<T>::SelfInvertAffine()
{
    Matrix3<T> rotScal = Matrix3<T>(rows[0].xyz(), rows[1].xyz(), rows[2].xyz());
    Matrix3<T> inv = rotScal.Inverted();
    
    Vector3<T> translation = rows[3].xyz();
    Vector3<T> invTrans = translation * inv;
    invTrans = -invTrans;
    
    rows[0] = Vector4<T>(inv.rows[0].xyz, T(0));
    rows[1] = Vector4<T>(inv.rows[1].xyz, T(0));
    rows[2] = Vector4<T>(inv.rows[2].xyz, T(0));
    rows[3] = Vector4<T>(invTrans,        T(1));
    return *this;
}

template <typename T>
float Matrix4<T>::Determinant(Matrix4 const& _m)
{
    return _m.Determinant(); 
}

template <typename T>
Matrix4<T> Matrix4<T>::Transpose(Matrix4 const& _m)
{
    return _m.Transposed();
}

template <typename T>
Matrix4<T> Matrix4<T>::Invert(Matrix4 const& _m)
{
    return _m.Inverted();
}

template <typename T>
Matrix4<T> Matrix4<T>::InvertAffine(Matrix4 const& _m)
{
    return _m.InvertedAffine();
}

template <typename T>
Matrix4<T> Matrix4<T>::MakeTranslation(Vector4<T> const& _v)
{
    return {
        {T(1), T(0), T(0), T(0)},
        {T(0), T(1), T(0), T(0)},
        {T(0), T(0), T(1), T(0)},
        {_v.x, _v.y, _v.z, T(1)}
    };
}

template <typename T>
Matrix4<T> Matrix4<T>::MakeScale(Vector4<T> const& _v)
{
    return {
        {_v.x, T(0), T(0), T(0)},
        {T(0), _v.x, T(0), T(0)},
        {T(0), T(0), _v.z, T(0)},
        {T(0), T(0), T(0), T(1)}
    };
}

template <typename T>
Matrix4<T> Matrix4<T>::MakeRotation(Vector4<T> const& _axis, T _angle)
{
    return {
            {T(1), T(0), T(0), T(0)},
            {T(0), T(1), T(0), T(0)},
            {T(0), T(0), T(1), T(0)},
            {T(0), T(0), T(0), T(1)}
    };
}

template <typename T>
Vector4<T> const& Matrix4<T>::operator[](int _i) const
{
    return rows[_i];
}

template <typename T>
Vector4<T>& Matrix4<T>::operator[](int _i)
{
    return rows[_i];
}

template <typename T>
T* Matrix4<T>::Data()
{
    return &m00;
}

template <typename T>
T const* Matrix4<T>::Data() const
{
    return &m00;
}



#endif
