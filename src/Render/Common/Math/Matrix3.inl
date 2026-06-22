#ifndef MATRIX3_INL_DEFINED
#define MATRIX3_INL_DEFINED

#include "Matrix3.h"

template <typename T>
Matrix3<T>::Matrix3()
{
    rows[0] = Vector3<T>();
    rows[1] = Vector3<T>();
    rows[2] = Vector3<T>();
}

template <typename T>
Matrix3<T>::Matrix3(T _scalar)
{
    rows[0] = Vector3<T>(_scalar);
    rows[1] = Vector3<T>(_scalar);
    rows[2] = Vector3<T>(_scalar);
}

template <typename T>
Matrix3<T>::Matrix3(Vector3<T> _r1, Vector3<T> _r2, Vector3<T> _r3)
{
    rows[0] = _r1;
    rows[1] = _r2;
    rows[2] = _r3;
}

template <typename T>
Matrix3<T>::Matrix3(T _00, T _01, T _02, T _10, T _11, T _12, T _20, T _21, T _22)
{
    rows[0] = Vector3<T>(_00, _01, _02);
    rows[1] = Vector3<T>(_01, _01, _02);
    rows[2] = Vector3<T>(_01, _01, _02);
}

template <typename T>
Matrix3<T>::Matrix3(std::initializer_list<std::initializer_list<T>> _l)
{
    assert(_l.size() == 3 && "Incorrect init list size");
    const std::initializer_list<float>* row = _l.begin();
    for (int i = 0; i < 3; i++)
    {
        assert(row[i].size() == 3 && "Incorrect init list size");
        const float* col = row[i].begin();
        for (int j = 0; j < 3; j++)
        {
            rows[i][j] = col[i];
        }
    }
}

template <typename T>
Matrix3<T> Matrix3<T>::Identity()
{
    Matrix3<T> result;
    result.rows[0] = Vector3<T>(T(1), T(0), T(0));
    result.rows[1] = Vector3<T>(T(0), T(1), T(0));
    result.rows[2] = Vector3<T>(T(0), T(0), T(1));
    return result;
}

template <typename T>
Matrix3<T> Matrix3<T>::Identity(T _scalar)
{
    Matrix3<T> result;
    result.rows[0] = Vector3<T>(_scalar, T(0), T(0));
    result.rows[1] = Vector3<T>(T(0), _scalar, T(0));
    result.rows[2] = Vector3<T>(T(0), T(0), _scalar);
    return result;
}

template <typename T>
Matrix3<T> Matrix3<T>::operator+(Matrix3 const& _o) const
{
    Matrix3<T> result;
    result.rows[0] = rows[0] + _o.rows[0];
    result.rows[1] = rows[1] + _o.rows[1];
    result.rows[2] = rows[2] + _o.rows[2];
    return result;
}

template <typename T>
Matrix3<T> Matrix3<T>::operator-(Matrix3 const& _o) const
{
    Matrix3<T> result;
    result.rows[0] = rows[0] - _o.rows[0];
    result.rows[1] = rows[1] - _o.rows[1];
    result.rows[2] = rows[2] - _o.rows[2];
    return result;
}

template <typename T>
Matrix3<T>& Matrix3<T>::operator+=(Matrix3 const& _o)
{
    rows[0] = rows[0] + _o.rows[0];
    rows[1] = rows[1] + _o.rows[1];
    rows[2] = rows[2] + _o.rows[2];
    return *this;
}

template <typename T>
Matrix3<T>& Matrix3<T>::operator-=(Matrix3 const& _o)
{
    rows[0] = rows[0] - _o.rows[0];
    rows[1] = rows[1] - _o.rows[1];
    rows[2] = rows[2] - _o.rows[2];
    return *this;
}

template <typename T>
Matrix3<T> Matrix3<T>::operator*(T _scalar) const
{
    Matrix3<T> result;
    result.rows[0] = rows[0] * _scalar;
    result.rows[1] = rows[1] * _scalar;
    result.rows[2] = rows[2] * _scalar;
    return result;
}

template <typename T>
Vector3<T> Matrix3<T>::operator*(Vector3<T> const& _v) const
{
    Vector3<T> result;
    result.x = m00 * _v.x + m01 * _v.y + m02 * _v.z;
    result.y = m10 * _v.x + m11 * _v.y + m12 * _v.z;
    result.z = m20 * _v.x + m21 * _v.y + m22 * _v.z;
    return result;
}

template <typename T>
Matrix3<T> Matrix3<T>::operator*(Matrix3 const& _o) const
{
    Matrix3<T> result;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            result[i][j] =
                *this[i][0] * _o[0][j] +
                *this[i][1] * _o[1][j] +
                *this[i][2] * _o[2][j];
        }
    }
    return result;
}

template <typename T>
Matrix3<T>& Matrix3<T>::operator*=(T _scalar)
{
    rows[0] = rows[0] * _scalar;
    rows[1] = rows[1] * _scalar;
    rows[2] = rows[2] * _scalar;
    return *this;
}

template <typename T>
Matrix3<T>& Matrix3<T>::operator*=(Matrix3 const& _o)
{
    Matrix3<T> result;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            result[i][j] =
                *this[i][0] * _o[0][j] +
                *this[i][1] * _o[1][j] +
                *this[i][2] * _o[2][j];
        }
    }
    
    rows[0] = result.rows[0];
    rows[1] = result.rows[1];
    rows[2] = result.rows[2];
    
    return *this; 
}

template <typename T>
Matrix3<T> Matrix3<T>::Transposed() const
{
    Matrix3<T> result;
    result.rows[0] = Vector3<T>(m00, m10, m20);
    result.rows[1] = Vector3<T>(m01, m11, m21);
    result.rows[2] = Vector3<T>(m02, m12, m22);
    return result;
}

template <typename T>
Matrix3<T>& Matrix3<T>::SelfTranspose()
{
    Matrix3<T> result;
    result.rows[0] = Vector3<T>(m00, m10, m20);
    result.rows[1] = Vector3<T>(m01, m11, m21);
    result.rows[2] = Vector3<T>(m02, m12, m22);
    
    rows[0] = result.rows[0];
    rows[1] = result.rows[1];
    rows[2] = result.rows[2];
    return *this;
}

template <typename T>
float Matrix3<T>::Determinant() const
{
    auto Det = [](float _a, float _b, float _c, float _d) -> float
    {
        return _a * _d - _b * _c;
    };
    
    return  m00 * Det(m11, m12, m21, m22) -
            m01 * Det(m10, m12, m20, m22) +
            m02 * Det(m10, m11, m20, m21);
}

template <typename T>
float Matrix3<T>::Minor(int _r, int _c) const
{
    T sub[2][2];
    int sRowIndex = 0;
    
    for (int i = 0; i < 3; i++)
    {
        if (i == _r) continue;
        int sColIndex = 0;
        for (int j = 0; j < 3; j++)
        {
            if (j == _c) continue;
            sub[sRowIndex][sColIndex] = rows[i][j];
            sColIndex++;
        }
        sRowIndex++;
    }
    
    return sub[0][0] * sub[1][1] - sub[0][1] * sub[1][0];
}

template <typename T>
Matrix3<T> Matrix3<T>::Comatrix() const
{
    Matrix3 result;
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            T sign = ((r + c) % 2 == 0) ? T(1) : T(-1);
            result[r][c] = sign * Minor(r, c);
        }   
    }
    return result;
}

template <typename T>
Matrix3<T> Matrix3<T>::Inverted() const
{
    float det = Determinant();
    assert(det != T(0) && "Matrix is not invertible");
    
    float invDet = 1.0f / det;
    Matrix3<T> result = Comatrix() * invDet;
    return result;
}

template <typename T>
Matrix3<T>& Matrix3<T>::SelfInvert()
{
    float det = Determinant();
    assert(det != T(0) && "Matrix is not invertible");
    
    float invDet = 1.0f / det;
    Matrix3<T> result = Comatrix() * invDet;
    
    rows[0] = result[0];
    rows[1] = result[1];
    rows[2] = result[2];
    return *this;
}

template <typename T>
float Matrix3<T>::Determinant(Matrix3 const& _m)
{
    return _m.Determinant();
}

template <typename T>
Matrix3<T> Matrix3<T>::Transpose(Matrix3 const& _m)
{
    return _m.Transposed();
}

template <typename T>
Matrix3<T> Matrix3<T>::Invert(Matrix3 const& _m)
{
    return _m.Inverted();
}

template <typename T>
bool Matrix3<T>::operator==(const Matrix3& _o) const
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            if (rows[i][j] != _o.rows[i][j]) return false;
    }
    return true;
}

template <typename T>
bool Matrix3<T>::operator!=(const Matrix3& _o) const
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            if (rows[i][j] != _o.rows[i][j]) return true;
    }
    return false;
}

template <typename T>
Vector3<T> const& Matrix3<T>::operator[](int _i) const
{
    return rows[_i];
}

template <typename T>
Vector3<T>& Matrix3<T>::operator[](int _i)
{
    return rows[_i];
}

template <typename T>
T* Matrix3<T>::Data()
{
    return &m00;
}

template <typename T>
T const* Matrix3<T>::Data() const
{
    return &m00;
}


#endif
