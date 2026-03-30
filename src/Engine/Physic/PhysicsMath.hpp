#pragma once
// ============================================================
//  PhysicsMath.hpp  –  Wrappers DirectXMath + utilitaires
// ============================================================
#include <DirectXMath.h>
#include <cmath>
#include <algorithm>

using namespace DirectX;

// ---- Scalaires ----
inline float Clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }
inline float Maxf(float a, float b) { return a > b ? a : b; }
inline float Minf(float a, float b) { return a < b ? a : b; }

// ---- XMFLOAT3 ops ----
inline XMFLOAT3 V3Add(const XMFLOAT3& a, const XMFLOAT3& b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline XMFLOAT3 V3Sub(const XMFLOAT3& a, const XMFLOAT3& b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline XMFLOAT3 V3Scale(const XMFLOAT3& a, float s)         { return {a.x*s,   a.y*s,   a.z*s}; }
inline XMFLOAT3 V3Neg(const XMFLOAT3& a)                    { return {-a.x, -a.y, -a.z}; }
inline float    V3Dot(const XMFLOAT3& a, const XMFLOAT3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float    V3LenSq(const XMFLOAT3& a)                  { return V3Dot(a,a); }
inline float    V3Len(const XMFLOAT3& a)                    { return sqrtf(V3LenSq(a)); }
inline XMFLOAT3 V3Cross(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}
inline XMFLOAT3 V3Normalize(const XMFLOAT3& a)
{
    float l = V3Len(a);
    if (l < 1e-8f) return {0,0,0};
    return V3Scale(a, 1.0f/l);
}
inline XMFLOAT3 V3CompMul(const XMFLOAT3& a, const XMFLOAT3& b) { return {a.x*b.x, a.y*b.y, a.z*b.z}; }
inline XMFLOAT3 V3Zero() { return {0,0,0}; }

// ---- XMFLOAT3x3 : tenseur inertie ----
struct Mat3
{
    float m[3][3] = {};

    static Mat3 Identity()
    {
        Mat3 r; r.m[0][0] = r.m[1][1] = r.m[2][2] = 1.0f; return r;
    }
    static Mat3 Diagonal(float ix, float iy, float iz)
    {
        Mat3 r; r.m[0][0]=ix; r.m[1][1]=iy; r.m[2][2]=iz; return r;
    }

    XMFLOAT3 Mul(const XMFLOAT3& v) const
    {
        return {
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        };
    }

    static Mat3 FromRotation(const XMFLOAT4& q)
    {
        XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&q));
        Mat3 r;
        XMFLOAT4X4 tmp;
        XMStoreFloat4x4(&tmp, rot);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i][j] = tmp.m[i][j];
        return r;
    }

    // R * D * R^T  (pour passer le tenseur en world space)
    static Mat3 SandwichDiag(const Mat3& R, const XMFLOAT3& diag)
    {
        Mat3 result;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
            {
                float s = 0;
                for (int k = 0; k < 3; ++k)
                    s += R.m[i][k] * diag_val(diag,k) * R.m[j][k];
                result.m[i][j] = s;
            }
        return result;
    }

    Mat3 Inverse3x3() const
    {
        // Inverse analytique 3x3
        float det =
            m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
           -m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
           +m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
        if (fabsf(det) < 1e-10f) return {};
        float inv = 1.0f / det;
        Mat3 r;
        r.m[0][0] = (m[1][1]*m[2][2]-m[1][2]*m[2][1])*inv;
        r.m[0][1] = (m[0][2]*m[2][1]-m[0][1]*m[2][2])*inv;
        r.m[0][2] = (m[0][1]*m[1][2]-m[0][2]*m[1][1])*inv;
        r.m[1][0] = (m[1][2]*m[2][0]-m[1][0]*m[2][2])*inv;
        r.m[1][1] = (m[0][0]*m[2][2]-m[0][2]*m[2][0])*inv;
        r.m[1][2] = (m[0][2]*m[1][0]-m[0][0]*m[1][2])*inv;
        r.m[2][0] = (m[1][0]*m[2][1]-m[1][1]*m[2][0])*inv;
        r.m[2][1] = (m[0][1]*m[2][0]-m[0][0]*m[2][1])*inv;
        r.m[2][2] = (m[0][0]*m[1][1]-m[0][1]*m[1][0])*inv;
        return r;
    }

private:
    static float diag_val(const XMFLOAT3& d, int k)
    { return k==0 ? d.x : k==1 ? d.y : d.z; }
};
