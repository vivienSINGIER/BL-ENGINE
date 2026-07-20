#ifndef MATRIX3_TEST_H_DEFINED
#define MATRIX3_TEST_H_DEFINED

#include "Test.hpp"
#include "../Core/Math/Matrix/Matrix3.h"
#include "../Core/Math/Matrix/Matrix4.h"
#include "../Core/Math/Vector/Vector3.h"
#include "../Core/Math/Quaternions/Quaternion.h"

// All expected values were computed with numpy and verified by hand.
// m_inv = [[2,1,0],[1,3,1],[0,1,2]]  det=8
// inv   = [[0.625,-0.25,0.125],[-0.25,0.5,-0.25],[0.125,-0.25,0.625]]
// cof   = [[5,-2,1],[-2,4,-2],[1,-2,5]]

static bool Mat3Equal(Matrix3<float> const& _a, Matrix3<float> const& _b)
{
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (!MathUtils::NearlyEqual(_a[r][c], _b[r][c], 1e-4f))
                return false;
    return true;
}

static bool FloatEqualM3(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-4f);
}

static bool Vec3EqualM3(Vector3<float> const& _a, Vector3<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f);
}

class TestMatrix3 : public Test
{
public:
    void Run() override
    {
        Test_Constructors();
        Test_Identity();
        Test_Arithmetic();
        Test_CompoundAssignment();
        Test_Comparison();
        Test_Transpose();
        Test_Determinant();
        Test_Minor();
        Test_Comatrix();
        Test_FrobeniusNorm();
        Test_Invert();
         Test_StaticMake();
         Test_Conversions();
        Test_Subscript();
        DisplayResults();
    }

private:

    using M3 = Matrix3<float>;
    using M4 = Matrix4<float>;
    using V3 = Vector3<float>;
    using Q  = Quaternion;

    // Element accessors
    static float GetM00(M3& m) { return m.m00; }
    static float GetM01(M3& m) { return m.m01; }
    static float GetM02(M3& m) { return m.m02; }
    static float GetM10(M3& m) { return m.m10; }
    static float GetM11(M3& m) { return m.m11; }
    static float GetM12(M3& m) { return m.m12; }
    static float GetM20(M3& m) { return m.m20; }
    static float GetM21(M3& m) { return m.m21; }
    static float GetM22(M3& m) { return m.m22; }

    static float GetM4_00(M4& m) { return m.m00; }
    static float GetM4_01(M4& m) { return m.m01; }
    static float GetM4_03(M4& m) { return m.m03; }
    static float GetM4_11(M4& m) { return m.m11; }
    static float GetM4_22(M4& m) { return m.m22; }
    static float GetM4_30(M4& m) { return m.m30; }
    static float GetM4_33(M4& m) { return m.m33; }

    static float Minor00(M3& m) { return m.Minor(0, 0); }
    static float Minor01(M3& m) { return m.Minor(0, 1); }
    static float Minor12(M3& m) { return m.Minor(1, 2); }
    static float Minor22(M3& m) { return m.Minor(2, 2); }

    static V3 GetRow0(M3& m) { return m[0]; }
    static V3 GetRow1(M3& m) { return m[1]; }
    static V3 GetRow2(M3& m) { return m[2]; }

    static float Data0(M3& m) { return m.Data()[0]; }
    static float Data1(M3& m) { return m.Data()[1]; }
    static float Data2(M3& m) { return m.Data()[2]; }
    static float Data3(M3& m) { return m.Data()[3]; }
    static float Data8(M3& m) { return m.Data()[8]; }

    // Test matrices
    // a = [[1,2,3],[4,5,6],[7,8,9]]  (singular)
    // b = [[9,8,7],[6,5,4],[3,2,1]]  (singular)
    // m = [[2,1,0],[1,3,1],[0,1,2]]  (invertible, det=8)
    static M3 A() { return M3(1,2,3, 4,5,6, 7,8,9); }
    static M3 B() { return M3(9,8,7, 6,5,4, 3,2,1); }
    static M3 M() { return M3(2,1,0, 1,3,1, 0,1,2); }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        M3 def;
        M3 scalar(3.0f);
        M3 rows(V3(1,2,3), V3(4,5,6), V3(7,8,9));
        M3 elems(1,2,3, 4,5,6, 7,8,9);
        M3 initList = { {1.0f,2.0f,3.0f}, {4.0f,5.0f,6.0f}, {7.0f,8.0f,9.0f} };

        // Default: all zeros
        TestValue("Constructor | default | m00",  &GetM00, FloatEqualM3, 0.0f, def);
        TestValue("Constructor | default | m11",  &GetM11, FloatEqualM3, 0.0f, def);
        TestValue("Constructor | default | m22",  &GetM22, FloatEqualM3, 0.0f, def);
        TestValue("Constructor | default | m01",  &GetM01, FloatEqualM3, 0.0f, def);

        // Scalar: diagonal = scalar, off-diagonal = 0
        TestValue("Constructor | scalar | m00",   &GetM00, FloatEqualM3, 3.0f, scalar);
        TestValue("Constructor | scalar | m11",   &GetM11, FloatEqualM3, 3.0f, scalar);
        TestValue("Constructor | scalar | m22",   &GetM22, FloatEqualM3, 3.0f, scalar);
        TestValue("Constructor | scalar | m01",   &GetM01, FloatEqualM3, 0.0f, scalar);
        TestValue("Constructor | scalar | m20",   &GetM20, FloatEqualM3, 0.0f, scalar);

        // Row constructor
        TestValue("Constructor | rows | m00",     &GetM00, FloatEqualM3, 1.0f, rows);
        TestValue("Constructor | rows | m01",     &GetM01, FloatEqualM3, 2.0f, rows);
        TestValue("Constructor | rows | m02",     &GetM02, FloatEqualM3, 3.0f, rows);
        TestValue("Constructor | rows | m10",     &GetM10, FloatEqualM3, 4.0f, rows);
        TestValue("Constructor | rows | m21",     &GetM21, FloatEqualM3, 8.0f, rows);
        TestValue("Constructor | rows | m22",     &GetM22, FloatEqualM3, 9.0f, rows);

        // Element constructor
        TestValue("Constructor | elems | m00",    &GetM00, FloatEqualM3, 1.0f, elems);
        TestValue("Constructor | elems | m01",    &GetM01, FloatEqualM3, 2.0f, elems);
        TestValue("Constructor | elems | m10",    &GetM10, FloatEqualM3, 4.0f, elems);
        TestValue("Constructor | elems | m12",    &GetM12, FloatEqualM3, 6.0f, elems);
        TestValue("Constructor | elems | m20",    &GetM20, FloatEqualM3, 7.0f, elems);
        TestValue("Constructor | elems | m22",    &GetM22, FloatEqualM3, 9.0f, elems);

        // Initializer list must match the element constructor
        TestObjectValue("Constructor | initList == elems", initList, &M3::operator==, true, elems);
        // Row and element constructors must agree
        TestObjectValue("Constructor | rows == elems",     rows,     &M3::operator==, true, elems);
    }

    // -----------------------------------------------------------------------

    void Test_Identity()
    {
        M3 ident  = M3::Identity();
        M3 ident5 = M3::Identity(5.0f);

        // Diagonal must be 1, all off-diagonal 0
        TestValue("Identity | m00", &GetM00, FloatEqualM3, 1.0f, ident);
        TestValue("Identity | m11", &GetM11, FloatEqualM3, 1.0f, ident);
        TestValue("Identity | m22", &GetM22, FloatEqualM3, 1.0f, ident);
        TestValue("Identity | m01", &GetM01, FloatEqualM3, 0.0f, ident);
        TestValue("Identity | m02", &GetM02, FloatEqualM3, 0.0f, ident);
        TestValue("Identity | m10", &GetM10, FloatEqualM3, 0.0f, ident);
        TestValue("Identity | m12", &GetM12, FloatEqualM3, 0.0f, ident);
        TestValue("Identity | m20", &GetM20, FloatEqualM3, 0.0f, ident);
        TestValue("Identity | m21", &GetM21, FloatEqualM3, 0.0f, ident);

        // Scalar identity: diagonal = 5, off-diagonal = 0
        TestValue("Identity | scalar | m00", &GetM00, FloatEqualM3, 5.0f, ident5);
        TestValue("Identity | scalar | m11", &GetM11, FloatEqualM3, 5.0f, ident5);
        TestValue("Identity | scalar | m22", &GetM22, FloatEqualM3, 5.0f, ident5);
        TestValue("Identity | scalar | m01", &GetM01, FloatEqualM3, 0.0f, ident5);
        TestValue("Identity | scalar | m12", &GetM12, FloatEqualM3, 0.0f, ident5);
        TestValue("Identity | scalar | m20", &GetM20, FloatEqualM3, 0.0f, ident5);

        // I * v = v  (non-trivial vector)
        V3 v(3.0f, -7.0f, 5.0f);
        TestObjectValue("Identity | I*v == v", ident,
            static_cast<V3(M3::*)(V3 const&) const>(&M3::operator*),
            Vec3EqualM3, v, v);

        // I * A = A
        M3 a = A();
        TestObjectValue("Identity | I*A == A", ident,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, a, a);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const M3 a = A(); // [[1,2,3],[4,5,6],[7,8,9]]
        const M3 b = B(); // [[9,8,7],[6,5,4],[3,2,1]]

        // a+b: all rows sum to 10
        TestObjectValue("operator+ | all 10s", a, &M3::operator+, Mat3Equal,
            M3(10,10,10, 10,10,10, 10,10,10), b);

        // a-b: [[-8,-6,-4],[-2,0,2],[4,6,8]]
        TestObjectValue("operator- | basic", a, &M3::operator-, Mat3Equal,
            M3(-8,-6,-4, -2,0,2, 4,6,8), b);

        // a*b (row-major): [[30,24,18],[84,69,54],[138,114,90]]
        TestObjectValue("operator* | matrix", a,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal,
            M3( 30, 24, 18,
                84, 69, 54,
               138,114, 90), b);

        // a * scalar 3: [[3,6,9],[12,15,18],[21,24,27]]
        TestObjectValue("operator* | scalar", a,
            static_cast<M3(M3::*)(float) const>(&M3::operator*),
            Mat3Equal,
            M3( 3, 6, 9,
               12,15,18,
               21,24,27), 3.0f);

        // a * v(1,2,3) = [1+4+9, 4+10+18, 7+16+27] = [14,32,50]
        V3 v(1.0f, 2.0f, 3.0f);
        TestObjectValue("operator* | vector | a", a,
            static_cast<V3(M3::*)(V3 const&) const>(&M3::operator*),
            Vec3EqualM3, V3(14.0f, 32.0f, 50.0f), v);

        // m * v(1,-1,2): row0=2-1+0=1, row1=1-3+2=0, row2=0-1+4=3
        const M3 m = M();
        V3 v2(1.0f, -1.0f, 2.0f);
        TestObjectValue("operator* | vector | m", m,
            static_cast<V3(M3::*)(V3 const&) const>(&M3::operator*),
            Vec3EqualM3, V3(1.0f, 0.0f, 3.0f), v2);

        // Non-commutative: a*b != b*a
        M3 ab = a * b;
        M3 ba = b * a;
        TestObjectValue("operator* | non-commutative", ab, &M3::operator!=, true, ba);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        const M3 a = A();
        const M3 b = B();

        M3 add = a;
        TestObjectSelf("operator+= | basic", add, &M3::operator+=, Mat3Equal,
            M3(10,10,10, 10,10,10, 10,10,10), b);

        M3 sub = a;
        TestObjectSelf("operator-= | basic", sub, &M3::operator-=, Mat3Equal,
            M3(-8,-6,-4, -2,0,2, 4,6,8), b);

        M3 mul = a;
        TestObjectSelf("operator*= | matrix", mul,
            static_cast<M3&(M3::*)(M3 const&)>(&M3::operator*=),
            Mat3Equal,
            M3( 30, 24, 18,
                84, 69, 54,
               138,114, 90), b);

        M3 scale = a;
        TestObjectSelf("operator*= | scalar", scale,
            static_cast<M3&(M3::*)(float)>(&M3::operator*=),
            Mat3Equal,
            M3( 3, 6, 9,
               12,15,18,
               21,24,27), 3.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const M3 a = A();
        const M3 b = A(); // same values
        const M3 c = B(); // different

        TestObjectValue("operator== | equal",     a, &M3::operator==, true,  b);
        TestObjectValue("operator== | not equal", a, &M3::operator==, false, c);
        TestObjectValue("operator!= | equal",     a, &M3::operator!=, false, b);
        TestObjectValue("operator!= | not equal", a, &M3::operator!=, true,  c);

        // Differ by one element only
        M3 almostA = A();
        almostA.m12 += 0.1f;
        TestObjectValue("operator== | single element differs", a, &M3::operator==, false, almostA);
        TestObjectValue("operator!= | single element differs", a, &M3::operator!=, true,  almostA);
    }

    // -----------------------------------------------------------------------

    void Test_Transpose()
    {
        const M3 a = A(); // [[1,2,3],[4,5,6],[7,8,9]]
        // aT = [[1,4,7],[2,5,8],[3,6,9]]
        const M3 expected(1,4,7, 2,5,8, 3,6,9);

        TestObjectValue("Transposed | member",       a, &M3::Transposed, Mat3Equal, expected);
        TestValue      ("Transpose  | static",       &M3::Transpose,    Mat3Equal, expected, a);

        M3 selfT = a;
        TestObjectSelf ("SelfTranspose | basic",     selfT, &M3::SelfTranspose, Mat3Equal, expected);

        // (aT)T = a
        TestObjectValue("Transposed | double",       a.Transposed(), &M3::Transposed, Mat3Equal, a);

        // Upper-triangular: [[1,2,3],[0,4,5],[0,0,6]] -> T = [[1,0,0],[2,4,0],[3,5,6]]
        M3 tri(1,2,3, 0,4,5, 0,0,6);
        M3 triT(1,0,0, 2,4,0, 3,5,6);
        TestObjectValue("Transposed | upper-tri",    tri, &M3::Transposed, Mat3Equal, triT);

        // m = [[2,1,0],[1,3,1],[0,1,2]] is symmetric: mT == m
        const M3 m = M();
        TestObjectValue("Transposed | symmetric m",  m, &M3::Transposed, Mat3Equal, m);
    }

    // -----------------------------------------------------------------------

    void Test_Determinant()
    {
        // m = [[2,1,0],[1,3,1],[0,1,2]], det = 2*(6-1) - 1*(2-0) + 0 = 8
        const M3 m = M();
        TestObjectValue("Determinant | member | m",   m, &M3::Determinant, FloatEqualM3, 8.0f);
        TestValue      ("Determinant | static | m",   static_cast<float(*)(M3 const&)>(&M3::Determinant),   FloatEqualM3, 8.0f, m);

        // Singular: det = 0
        const M3 a = A();
        TestObjectValue("Determinant | singular",     a, &M3::Determinant, FloatEqualM3, 0.0f);

        // Diagonal [[3,0,0],[0,5,0],[0,0,7]]: det = 3*5*7 = 105
        M3 diag(3,0,0, 0,5,0, 0,0,7);
        TestObjectValue("Determinant | diagonal",     diag, &M3::Determinant, FloatEqualM3, 105.0f);

        // det(2*m) = 2^3 * det(m) = 8 * 8 = 64
        M3 m2 = m * 2.0f;
        TestObjectValue("Determinant | det(2m)=8*det(m)", m2, &M3::Determinant, FloatEqualM3, 64.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Minor()
    {
        // m = [[2,1,0],[1,3,1],[0,1,2]]
        // minor(0,0) = det([[3,1],[1,2]]) = 6-1 = 5
        // minor(0,1) = det([[1,1],[0,2]]) = 2-0 = 2
        // minor(1,2) = det([[2,1],[0,1]]) = 2-0 = 2
        // minor(2,2) = det([[2,1],[1,3]]) = 6-1 = 5
        M3 m = M();
        TestValue("Minor | (0,0)", &Minor00, FloatEqualM3, 5.0f, m);
        TestValue("Minor | (0,1)", &Minor01, FloatEqualM3, 2.0f, m);
        TestValue("Minor | (1,2)", &Minor12, FloatEqualM3, 2.0f, m);
        TestValue("Minor | (2,2)", &Minor22, FloatEqualM3, 5.0f, m);
    }

    // -----------------------------------------------------------------------

    void Test_Comatrix()
    {
        // m = [[2,1,0],[1,3,1],[0,1,2]]
        // cof[i][j] = (-1)^(i+j) * minor(i,j)
        // cof = [[5,-2,1],[-2,4,-2],[1,-2,5]]
        const M3 m = M();
        const M3 expected(5,-2,1, -2,4,-2, 1,-2,5);
        TestObjectValue("Comatrix | m", m, &M3::Comatrix, Mat3Equal, expected);

        // m * adj(m) = det(m) * I = 8 * I
        // adj = cof^T; since m is symmetric here, adj == cof
        M3 adj     = m.Comatrix().Transposed();
        M3 product = m * adj;
        TestObjectValue("Comatrix | m*adj(m) = 8*I", product, &M3::operator==, true, M3::Identity(8.0f));
    }

    // -----------------------------------------------------------------------

    void Test_FrobeniusNorm()
    {
        // m = [[2,1,0],[1,3,1],[0,1,2]]
        // ||m||_F = sqrt(4+1+0+1+9+1+0+1+4) = sqrt(21)
        const M3 m = M();
        TestObjectValue("FrobeniusNorm | m",        m, &M3::FrobeniusNorm, FloatEqualM3, std::sqrt(21.0f));

        // Diagonal [[3,0,0],[0,5,0],[0,0,7]]: norm = sqrt(9+25+49) = sqrt(83)
        M3 diag(3,0,0, 0,5,0, 0,0,7);
        TestObjectValue("FrobeniusNorm | diagonal", diag, &M3::FrobeniusNorm, FloatEqualM3, std::sqrt(83.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Invert()
    {
        // m = [[2,1,0],[1,3,1],[0,1,2]], det=8
        // inv = [[0.625,-0.25,0.125],[-0.25,0.5,-0.25],[0.125,-0.25,0.625]]
        const M3 m = M();
        const M3 expectedInv( 0.625f, -0.25f,  0.125f,
                              -0.25f,  0.5f,   -0.25f,
                               0.125f,-0.25f,   0.625f);

        TestObjectValue("Inverted | member",       m, &M3::Inverted, Mat3Equal, expectedInv);
        TestValue      ("Invert   | static",       &M3::Invert,     Mat3Equal, expectedInv, m);

        // m * inv(m) = I
        M3 product  = m * m.Inverted();
        TestObjectValue("Inverted | m*inv(m) = I", product,      &M3::operator==, true, M3::Identity());

        // inv(m) * m = I
        M3 productL = m.Inverted() * m;
        TestObjectValue("Inverted | inv(m)*m = I", productL,     &M3::operator==, true, M3::Identity());

        // inv(inv(m)) = m
        TestObjectValue("Inverted | double",       m.Inverted(), &M3::Inverted,   Mat3Equal, m);

        // SelfInvert
        M3 selfInv = m;
        TestObjectSelf("SelfInvert | m",           selfInv, &M3::SelfInvert, Mat3Equal, expectedInv);

        // Diagonal [[3,0,0],[0,5,0],[0,0,7]]: inv = diag(1/3,1/5,1/7)
        M3 diag(3,0,0, 0,5,0, 0,0,7);
        M3 diagExpected(1.0f/3.0f,0,0, 0,1.0f/5.0f,0, 0,0,1.0f/7.0f);
        TestObjectValue("Inverted | diagonal",     diag, &M3::Inverted, Mat3Equal, diagExpected);
    }

    // -----------------------------------------------------------------------

    void Test_StaticMake()
    {
        constexpr float PI4 = 3.14159265358979f / 4.0f;
        constexpr float PI2 = 3.14159265358979f / 2.0f;

        // ============================================================
        // MakeScale
        // ============================================================
        M3 s = M3::MakeScale(V3(2.0f, 3.0f, 4.0f));
        TestValue("MakeScale | m00", &GetM00, FloatEqualM3, 2.0f, s);
        TestValue("MakeScale | m11", &GetM11, FloatEqualM3, 3.0f, s);
        TestValue("MakeScale | m22", &GetM22, FloatEqualM3, 4.0f, s);
        TestValue("MakeScale | m01", &GetM01, FloatEqualM3, 0.0f, s);
        TestValue("MakeScale | m10", &GetM10, FloatEqualM3, 0.0f, s);

        // S * v = (sx*vx, sy*vy, sz*vz)
        V3 v(1.0f, 2.0f, 3.0f);
        TestObjectValue("MakeScale | S*v", s,
            static_cast<V3(M3::*)(V3 const&) const>(&M3::operator*),
            Vec3EqualM3, V3(2.0f, 6.0f, 12.0f), v);

        // ============================================================
        // MakeRotationZ (45 deg)
        // row-vector convention: m00=cos, m01=sin, m10=-sin, m11=cos, m22=1
        // ============================================================
        float c   = std::cos(PI4);   // ~0.70711
        float s45 = std::sin(PI4);   // ~0.70711
        M3 rz45 = M3::MakeRotationZ(PI4);
        TestValue("MakeRotationZ | 45 | m00",  &GetM00, FloatEqualM3,  c,    rz45);
        TestValue("MakeRotationZ | 45 | m01",  &GetM01, FloatEqualM3,  s45,  rz45);
        TestValue("MakeRotationZ | 45 | m10",  &GetM10, FloatEqualM3, -s45,  rz45);
        TestValue("MakeRotationZ | 45 | m11",  &GetM11, FloatEqualM3,  c,    rz45);
        TestValue("MakeRotationZ | 45 | m22",  &GetM22, FloatEqualM3,  1.0f, rz45);

        // (1,0,0) * Rz45 = (cos45, sin45, 0)  -- row vector on the left
        V3 ex(1.0f, 0.0f, 0.0f);
        TestObjectValue("MakeRotationZ | 45 | (1,0,0)*Rz", ex,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(c, s45, 0.0f), rz45);

        // ============================================================
        // MakeRotationX (90 deg): (0,1,0) * Rx90 = (0,0,-1) for row-vector
        // convention defined as Rx = [[1,0,0],[0,c,s],[0,-s,c]]
        // ============================================================
        M3 rx90 = M3::MakeRotationX(PI2);
        V3 ey(0.0f, 1.0f, 0.0f);
        TestObjectValue("MakeRotationX | 90 | (0,1,0)*Rx", ey,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(0.0f, 0.0f, 1.0f), rx90);

        // ============================================================
        // MakeRotationY (90 deg): (0,0,1) * Ry90 = (1,0,0)
        // ============================================================
        M3 ry90 = M3::MakeRotationY(PI2);
        V3 ez(0.0f, 0.0f, 1.0f);
        TestObjectValue("MakeRotationY | 90 | (0,0,1)*Ry", ez,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(1.0f, 0.0f, 0.0f), ry90);

        // ============================================================
        // All single-axis rotations are orthogonal: R * R^T = I
        // ============================================================
        TestObjectValue("MakeRotationZ | 45 | R*RT = I", rz45,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rz45.Transposed());
        TestObjectValue("MakeRotationX | 90 | R*RT = I", rx90,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rx90.Transposed());
        TestObjectValue("MakeRotationY | 90 | R*RT = I", ry90,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), ry90.Transposed());

        // ============================================================
        // MakeRotationXYZ(angleX, angleY, angleZ)
        // Must equal Rx(angleX) * Ry(angleY) * Rz(angleZ) -- X applied first,
        // then Y, then Z, for row vectors (v' = v * R).
        // ============================================================
        M3 rxyz_expected = M3::MakeRotationXYZ(0.3f, 0.5f, 0.7f);
        M3 rxyz = M3::MakeRotationX(0.3f) * M3::MakeRotationY(0.5f);
        M3 rz = M3::MakeRotationZ(0.7f);
        TestObjectValue("MakeRotationXYZ | matches Rx*Ry*Rz", 
            rxyz, static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*), 
            rxyz_expected, rz);

        // det=1 and orthogonal
        TestObjectValue("MakeRotationXYZ | det=1", rxyz, &M3::Determinant, FloatEqualM3, 1.0f);
        TestObjectValue("MakeRotationXYZ | R*RT=I", rxyz,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rxyz.Transposed());

        // Independent ground-truth vector check (computed externally, not from the formula above):
        // v=(1,2,3) * MakeRotationXYZ(0.3, 0.5, 0.7) ~= (1.27910878, 2.41636365, 2.55442115)
        V3 vxyz(1.0f, 2.0f, 3.0f);
        TestObjectValue("MakeRotationXYZ | known vector", vxyz,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(1.27910878f, 2.41636365f, 2.55442115f), rxyz_expected);

        // ============================================================
        // MakeRotationZYX(angleZ, angleY, angleX)
        // Must equal Rz(angleZ) * Ry(angleY) * Rx(angleX) -- Z applied first,
        // then Y, then X, for row vectors.
        // ============================================================
        M3 rzyx = M3::MakeRotationZYX(0.3f, 0.5f, 0.7f);
        M3 rzyx_expected = M3::MakeRotationZ(0.3f) * M3::MakeRotationY(0.5f) * M3::MakeRotationX(0.7f);
        TestObjectValue("MakeRotationZYX | matches Rz*Ry*Rx", rzyx,
            &M3::operator==, true, rzyx_expected);

        TestObjectValue("MakeRotationZYX | det=1", rzyx, &M3::Determinant, FloatEqualM3, 1.0f);
        TestObjectValue("MakeRotationZYX | R*RT=I", rzyx,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rzyx.Transposed());

        // Independent ground-truth vector check:
        // v=(1,2,3) * MakeRotationZYX(0.3, 0.5, 0.7) ~= (1.7579765, 0.10384145, 3.30132331)
        V3 vzyx(1.0f, 2.0f, 3.0f);
        TestObjectValue("MakeRotationZYX | known vector", vzyx,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(1.7579765f, 0.10384145f, 3.30132331f), rzyx);

        // Sanity: XYZ and ZYX must NOT be equal in general (different composition order)
        TestObjectValue("MakeRotationXYZ | != MakeRotationZYX", rxyz,
            &M3::operator==, false, rzyx);

        // ============================================================
        // MakeRotationYPR(yawY, pitchX, rollZ)
        // Must equal Ry(yaw) * Rx(pitch) * Rz(roll) -- Yaw applied first,
        // then Pitch, then Roll, for row vectors.
        // ============================================================
        M3 rypr = M3::MakeRotationYPR(0.3f, 0.5f, 0.7f);
        M3 rypr_expected = M3::MakeRotationY(0.3f) * M3::MakeRotationX(0.5f) * M3::MakeRotationZ(0.7f);
        TestObjectValue("MakeRotationYPR | matches Ry*Rx*Rz", rypr,
            &M3::operator==, true, rypr_expected);

        TestObjectValue("MakeRotationYPR | det=1", rypr, &M3::Determinant, FloatEqualM3, 1.0f);
        TestObjectValue("MakeRotationYPR | R*RT=I", rypr,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rypr.Transposed());

        // Independent ground-truth vector check:
        // v=(1,2,3) * MakeRotationYPR(yaw=0.3, pitch=0.5, roll=0.7) ~= (1.07195915, 1.58644749, 3.21466763)
        V3 vypr(1.0f, 2.0f, 3.0f);
        TestObjectValue("MakeRotationYPR | known vector", vypr,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(1.07195915f, 1.58644749f, 3.21466763f), rypr);

        // ============================================================
        // MakeRotationRPY(rollZ, pitchX, yawY)
        // Must equal Rz(roll) * Rx(pitch) * Ry(yaw) -- Roll applied first,
        // then Pitch, then Yaw, for row vectors.
        // ============================================================
        M3 rrpy = M3::MakeRotationRPY(0.3f, 0.5f, 0.7f);
        M3 rrpy_expected = M3::MakeRotationZ(0.3f) * M3::MakeRotationX(0.5f) * M3::MakeRotationY(0.7f);
        TestObjectValue("MakeRotationRPY | matches Rz*Rx*Ry", rrpy,
            &M3::operator==, true, rrpy_expected);

        TestObjectValue("MakeRotationRPY | det=1", rrpy, &M3::Determinant, FloatEqualM3, 1.0f);
        TestObjectValue("MakeRotationRPY | R*RT=I", rrpy,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rrpy.Transposed());

        // Independent ground-truth vector check:
        // v=(1,2,3) * MakeRotationRPY(roll=0.3, pitch=0.5, yaw=0.7) ~= (2.65608413, 0.49784005, 2.5879282)
        V3 vrpy(1.0f, 2.0f, 3.0f);
        TestObjectValue("MakeRotationRPY | known vector", vrpy,
            static_cast<V3(V3::*)(M3 const&) const>(&V3::operator*),
            Vec3EqualM3, V3(2.65608413f, 0.49784005f, 2.5879282f), rrpy);

        // Sanity: YPR and RPY must NOT be equal in general (different composition order)
        TestObjectValue("MakeRotationYPR | != MakeRotationRPY", rypr,
            &M3::operator==, false, rrpy);

        // ============================================================
        // MakeRotation(axis, angle): generic axis-angle must match the
        // dedicated single-axis builders for the X, Y, and Z basis axes.
        // ============================================================
        M3 rotAxisX = M3::MakeRotation(V3(1, 0, 0), PI2);
        TestObjectValue("MakeRotation | axis X 90 == MakeRotationX", rotAxisX,
            &M3::operator==, true, rx90);

        M3 rotAxisY = M3::MakeRotation(V3(0, 1, 0), PI2);
        TestObjectValue("MakeRotation | axis Y 90 == MakeRotationY", rotAxisY,
            &M3::operator==, true, ry90);

        M3 rotAxisZ = M3::MakeRotation(V3(0, 0, 1), PI4);
        TestObjectValue("MakeRotation | axis Z 45 == MakeRotationZ", rotAxisZ,
            &M3::operator==, true, rz45);

        // MakeRotation with an arbitrary (non-axis-aligned) axis must still be
        // orthogonal with det=1, and rotating the axis itself should be a no-op.
        V3 arbitraryAxis = V3(1.0f, 1.0f, 1.0f).Normalized();
        M3 rotArbitrary = M3::MakeRotation(arbitraryAxis, PI4);
        TestObjectValue("MakeRotation | arbitrary axis | det=1", rotArbitrary,
            &M3::Determinant, FloatEqualM3, 1.0f);
        TestObjectValue("MakeRotation | arbitrary axis | R*RT=I", rotArbitrary,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), rotArbitrary.Transposed());
        TestObjectValue("MakeRotation | arbitrary axis | axis*R == axis", rotArbitrary,
            static_cast<V3(M3::*)(V3 const&) const>(&M3::operator*),
            Vec3EqualM3, arbitraryAxis, arbitraryAxis);

        // ============================================================
        // MakeRotationQuat: must match the equivalent matrix builders
        // for several known rotations, not just one axis.
        // ============================================================
        Q q90Z = Q::FromAxisAngle(V3(0, 0, 1), PI2);
        M3 fromQuatZ = M3::MakeRotationQuat(q90Z);
        M3 rotZ90 = M3::MakeRotationZ(PI2);
        
        TestValue("MakeRotationQuat | 90 deg Z == MakeRotationZ",
            &Mat3Equal, true, rotZ90, fromQuatZ);

        Q q90X = Q::FromAxisAngle(V3(1, 0, 0), PI2);
        M3 fromQuatX = M3::MakeRotationQuat(q90X);
        TestValue("MakeRotationQuat | 90 deg X == MakeRotationX",
            &Mat3Equal, true, rx90, fromQuatX);

        Q q90Y = Q::FromAxisAngle(V3(0, 1, 0), PI2);
        M3 fromQuatY = M3::MakeRotationQuat(q90Y);
        TestValue("MakeRotationQuat | 90 deg Y == MakeRotationY",
            &Mat3Equal, true, ry90, fromQuatY);

        // Identity quaternion must produce the identity matrix
        Q qIdentity = Q::FromAxisAngle(V3(0, 0, 1), 0.0f);
        M3 fromQuatIdentity = M3::MakeRotationQuat(qIdentity);
        TestObjectValue("MakeRotationQuat | identity quat == Identity", fromQuatIdentity,
            &M3::operator==, true, M3::Identity());
    }

    // -----------------------------------------------------------------------

    void Test_Conversions()
    {
        // ToMatrix4: upper-left 3x3 of result must match source
        // m = [[2,1,0],[1,3,1],[0,1,2]]
        const M3 m = M();
        M4 m4 = m.ToMatrix4();
        TestValue("ToMatrix4 | m00", &GetM4_00, FloatEqualM3, 2.0f, m4);
        TestValue("ToMatrix4 | m01", &GetM4_01, FloatEqualM3, 1.0f, m4);
        TestValue("ToMatrix4 | m11", &GetM4_11, FloatEqualM3, 3.0f, m4);
        TestValue("ToMatrix4 | m22", &GetM4_22, FloatEqualM3, 2.0f, m4);
        // Homogeneous padding: bottom-right = 1, translation column = 0
        TestValue("ToMatrix4 | m33", &GetM4_33, FloatEqualM3, 1.0f, m4);
        TestValue("ToMatrix4 | m03", &GetM4_03, FloatEqualM3, 0.0f, m4);
        TestValue("ToMatrix4 | m30", &GetM4_30, FloatEqualM3, 0.0f, m4);

        // ToQuaternion: rotation matrix -> round-trip back via MakeRotationQuat
        float pi2 = 3.14159265358979f / 2.0f;
        M3 rz = M3::MakeRotationZ(pi2);
        Q  q  = rz.ToQuaternion();
        M3 back = M3::MakeRotationQuat(q);
        TestValue("ToQuaternion | round-trip Rz90", &Mat3Equal, true, back, rz);
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        M3 m = A(); // [[1,2,3],[4,5,6],[7,8,9]]

        TestValue("operator[] | row0", &GetRow0, Vec3EqualM3, V3(1,2,3), m);
        TestValue("operator[] | row1", &GetRow1, Vec3EqualM3, V3(4,5,6), m);
        TestValue("operator[] | row2", &GetRow2, Vec3EqualM3, V3(7,8,9), m);

        // Data() linear layout must be row-major: m00,m01,m02,m10,...
        TestValue("Data() | [0] = m00", &Data0, FloatEqualM3, 1.0f, m);
        TestValue("Data() | [1] = m01", &Data1, FloatEqualM3, 2.0f, m);
        TestValue("Data() | [2] = m02", &Data2, FloatEqualM3, 3.0f, m);
        TestValue("Data() | [3] = m10", &Data3, FloatEqualM3, 4.0f, m);
        TestValue("Data() | [8] = m22", &Data8, FloatEqualM3, 9.0f, m);
    }
};

#endif