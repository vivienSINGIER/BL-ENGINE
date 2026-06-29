#ifndef MATRIX4_TEST_H_DEFINED
#define MATRIX4_TEST_H_DEFINED

#include "Test.hpp"
#include "../Render/Common/Math/Matrix/Matrix3.h"
#include "../Render/Common/Math/Matrix/Matrix4.h"
#include "../Render/Common/Math/Vector/Vector3.h"
#include "../Render/Common/Math/Vector/Vector4.h"
#include "../Render/Common/Math/Quaternions/Quaternion.h"

// All expected values were computed with numpy and verified by hand.
//
// Invertible test matrix M (det != 0):
//   m = [[2,0,0,0],[0,3,0,0],[0,0,1,0],[1,2,3,1]]  (affine, lower-triangular-ish)
//
// For det/inversion tests we use a denser matrix P:
//   p = [[2,1,0,0],[1,3,1,0],[0,1,2,0],[0,0,0,5]]
//   det(p) = 5 * det([[2,1,0],[1,3,1],[0,1,2]]) = 5 * 8 = 40
//   inv(p) upper-left 3x3 = inv([[2,1,0],[1,3,1],[0,1,2]]) / 1  (block-diagonal)
//         = [[0.625,-0.25,0.125],[-0.25,0.5,-0.25],[0.125,-0.25,0.625]]
//   inv(p) m33 = 1/5 = 0.2

static bool Mat4Equal(Matrix4<float> const& _a, Matrix4<float> const& _b)
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            if (!MathUtils::NearlyEqual(_a[r][c], _b[r][c], 1e-4f))
                return false;
    return true;
}

static bool FloatEqualM4(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-4f);
}

static bool FloatNotEqualM4(float const& _a, float const& _b)
{
    return !MathUtils::NearlyEqual(_a, _b, 1e-4f);
}

static bool Vec3EqualM4(Vector3<float> const& _a, Vector3<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f);
}

// static bool Vec4Equal(Vector4<float> const& _a, Vector4<float> const& _b)
// {
//     return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
//            MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
//            MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f) &&
//            MathUtils::NearlyEqual(_a.w, _b.w, 1e-4f);
// }

class TestMatrix4 : public Test
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
        Test_Invert();
        Test_InvertAffine();
        Test_StaticMake();
        Test_Conversions();
        Test_Subscript();
        DisplayResults();
    }

private:

    using M3 = Matrix3<float>;
    using M4 = Matrix4<float>;
    using V3 = Vector3<float>;
    using V4 = Vector4<float>;
    using Q  = Quaternion;

    // -----------------------------------------------------------------------
    // Element accessors
    // -----------------------------------------------------------------------
    static float GetM00(M4& m) { return m.m00; }
    static float GetM01(M4& m) { return m.m01; }
    static float GetM02(M4& m) { return m.m02; }
    static float GetM03(M4& m) { return m.m03; }
    static float GetM10(M4& m) { return m.m10; }
    static float GetM11(M4& m) { return m.m11; }
    static float GetM12(M4& m) { return m.m12; }
    static float GetM13(M4& m) { return m.m13; }
    static float GetM20(M4& m) { return m.m20; }
    static float GetM21(M4& m) { return m.m21; }
    static float GetM22(M4& m) { return m.m22; }
    static float GetM23(M4& m) { return m.m23; }
    static float GetM30(M4& m) { return m.m30; }
    static float GetM31(M4& m) { return m.m31; }
    static float GetM32(M4& m) { return m.m32; }
    static float GetM33(M4& m) { return m.m33; }

    static float GetM3_M00(M3& m) { return m.m00; }
    static float GetM3_M01(M3& m) { return m.m01; }
    static float GetM3_M02(M3& m) { return m.m02; }
    static float GetM3_M10(M3& m) { return m.m10; }
    static float GetM3_M11(M3& m) { return m.m11; }
    static float GetM3_M12(M3& m) { return m.m12; }
    static float GetM3_M20(M3& m) { return m.m20; }
    static float GetM3_M21(M3& m) { return m.m21; }
    static float GetM3_M22(M3& m) { return m.m22; }
    
    static float Minor00(M4& m) { return m.Minor(0, 0); }
    static float Minor03(M4& m) { return m.Minor(0, 3); }
    static float Minor11(M4& m) { return m.Minor(1, 1); }
    static float Minor33(M4& m) { return m.Minor(3, 3); }

    static V4 GetRow0(M4& m) { return m[0]; }
    static V4 GetRow1(M4& m) { return m[1]; }
    static V4 GetRow2(M4& m) { return m[2]; }
    static V4 GetRow3(M4& m) { return m[3]; }

    static float Data0 (M4& m) { return m.Data()[0];  }
    static float Data1 (M4& m) { return m.Data()[1];  }
    static float Data4 (M4& m) { return m.Data()[4];  }
    static float Data15(M4& m) { return m.Data()[15]; }

    // -----------------------------------------------------------------------
    // Test matrices
    //
    //  A = [[1,2,3,4],[5,6,7,8],[9,10,11,12],[13,14,15,16]]  (singular, rank 2)
    //  B = [[16,15,14,13],[12,11,10,9],[8,7,6,5],[4,3,2,1]]  (singular)
    //  P = [[2,1,0,0],[1,3,1,0],[0,1,2,0],[0,0,0,5]]         (block-diagonal, invertible, det=40)
    // -----------------------------------------------------------------------
    static M4 A()
    {
        return M4( 1, 2, 3, 4,
                   5, 6, 7, 8,
                   9,10,11,12,
                  13,14,15,16);
    }
    static M4 B()
    {
        return M4(16,15,14,13,
                  12,11,10, 9,
                   8, 7, 6, 5,
                   4, 3, 2, 1);
    }
    // Block-diagonal invertible matrix: upper-left = M3-test's M, lower-right = 5
    static M4 P()
    {
        return M4(2,1,0,0,
                  1,3,1,0,
                  0,1,2,0,
                  0,0,0,5);
    }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        M4 def;
        M4 scalar(3.0f);
        M4 rows(V4(1,2,3,4), V4(5,6,7,8), V4(9,10,11,12), V4(13,14,15,16));
        M4 elems( 1, 2, 3, 4,
                  5, 6, 7, 8,
                  9,10,11,12,
                 13,14,15,16);
        M4 initList = { {1.0f,2.0f,3.0f,4.0f},
                        {5.0f,6.0f,7.0f,8.0f},
                        {9.0f,10.0f,11.0f,12.0f},
                        {13.0f,14.0f,15.0f,16.0f} };

        // Default: all zeros
        TestValue("Constructor | default | m00",  &GetM00, FloatEqualM4, 0.0f, def);
        TestValue("Constructor | default | m11",  &GetM11, FloatEqualM4, 0.0f, def);
        TestValue("Constructor | default | m33",  &GetM33, FloatEqualM4, 0.0f, def);
        TestValue("Constructor | default | m03",  &GetM03, FloatEqualM4, 0.0f, def);

        // Scalar: diagonal = scalar, off-diagonal = 0
        TestValue("Constructor | scalar | m00",   &GetM00, FloatEqualM4, 3.0f, scalar);
        TestValue("Constructor | scalar | m11",   &GetM11, FloatEqualM4, 3.0f, scalar);
        TestValue("Constructor | scalar | m22",   &GetM22, FloatEqualM4, 3.0f, scalar);
        TestValue("Constructor | scalar | m33",   &GetM33, FloatEqualM4, 3.0f, scalar);
        TestValue("Constructor | scalar | m01",   &GetM01, FloatEqualM4, 0.0f, scalar);
        TestValue("Constructor | scalar | m30",   &GetM30, FloatEqualM4, 0.0f, scalar);

        // Row constructor
        TestValue("Constructor | rows | m00",     &GetM00, FloatEqualM4,  1.0f, rows);
        TestValue("Constructor | rows | m03",     &GetM03, FloatEqualM4,  4.0f, rows);
        TestValue("Constructor | rows | m10",     &GetM10, FloatEqualM4,  5.0f, rows);
        TestValue("Constructor | rows | m33",     &GetM33, FloatEqualM4, 16.0f, rows);

        // Element constructor
        TestValue("Constructor | elems | m00",    &GetM00, FloatEqualM4,  1.0f, elems);
        TestValue("Constructor | elems | m03",    &GetM03, FloatEqualM4,  4.0f, elems);
        TestValue("Constructor | elems | m10",    &GetM10, FloatEqualM4,  5.0f, elems);
        TestValue("Constructor | elems | m23",    &GetM23, FloatEqualM4, 12.0f, elems);
        TestValue("Constructor | elems | m30",    &GetM30, FloatEqualM4, 13.0f, elems);
        TestValue("Constructor | elems | m33",    &GetM33, FloatEqualM4, 16.0f, elems);

        // Initializer list must match element constructor
        TestObjectValue("Constructor | initList == elems", initList, &M4::operator==, true, elems);
        // Row and element constructors must agree
        TestObjectValue("Constructor | rows == elems",     rows,     &M4::operator==, true, elems);
    }

    // -----------------------------------------------------------------------

    void Test_Identity()
    {
        M4 ident  = M4::Identity();
        M4 ident5 = M4::Identity(5.0f);

        // Diagonal must be 1, all off-diagonal 0
        TestValue("Identity | m00", &GetM00, FloatEqualM4, 1.0f, ident);
        TestValue("Identity | m11", &GetM11, FloatEqualM4, 1.0f, ident);
        TestValue("Identity | m22", &GetM22, FloatEqualM4, 1.0f, ident);
        TestValue("Identity | m33", &GetM33, FloatEqualM4, 1.0f, ident);
        TestValue("Identity | m01", &GetM01, FloatEqualM4, 0.0f, ident);
        TestValue("Identity | m03", &GetM03, FloatEqualM4, 0.0f, ident);
        TestValue("Identity | m30", &GetM30, FloatEqualM4, 0.0f, ident);
        TestValue("Identity | m12", &GetM12, FloatEqualM4, 0.0f, ident);

        // Scalar identity: diagonal = 5, off-diagonal = 0
        TestValue("Identity | scalar | m00", &GetM00, FloatEqualM4, 5.0f, ident5);
        TestValue("Identity | scalar | m11", &GetM11, FloatEqualM4, 5.0f, ident5);
        TestValue("Identity | scalar | m22", &GetM22, FloatEqualM4, 5.0f, ident5);
        TestValue("Identity | scalar | m33", &GetM33, FloatEqualM4, 5.0f, ident5);
        TestValue("Identity | scalar | m01", &GetM01, FloatEqualM4, 0.0f, ident5);
        TestValue("Identity | scalar | m13", &GetM13, FloatEqualM4, 0.0f, ident5);

        // I * v = v  (non-trivial vector)
        V4 v(3.0f, -7.0f, 5.0f, 1.0f);
        TestObjectValue("Identity | I*v4 == v4", ident,
            static_cast<V4(M4::*)(V4 const&) const>(&M4::operator*),
            Vec4Equal, v, v);

        // I * A = A
        M4 a = A();
        TestObjectValue("Identity | I*A == A", ident,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, a, a);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const M4 a = A(); // all rows {1..4}, {5..8}, {9..12}, {13..16}
        const M4 b = B(); // all rows {16..13}, {12..9}, {8..5}, {4..1}

        // a+b: row i+j sums elementwise to 17 everywhere
        TestObjectValue("operator+ | all 17s", a, &M4::operator+, Mat4Equal,
            M4(17,17,17,17,  17,17,17,17,  17,17,17,17,  17,17,17,17), b);

        // a-b: row0: {1-16,2-15,3-14,4-13}={-15,-13,-11,-9}
        //      row1: {5-12,6-11,7-10,8-9}={-7,-5,-3,-1}
        //      row2: {9-8,10-7,11-6,12-5}={1,3,5,7}
        //      row3: {13-4,14-3,15-2,16-1}={9,11,13,15}
        TestObjectValue("operator- | basic", a, &M4::operator-, Mat4Equal,
            M4(-15,-13,-11,-9,
                -7, -5, -3,-1,
                 1,  3,  5, 7,
                 9, 11, 13,15), b);

        // a * scalar 2
        TestObjectValue("operator* | scalar", a,
            static_cast<M4(M4::*)(float) const>(&M4::operator*),
            Mat4Equal,
            M4( 2, 4, 6, 8,
               10,12,14,16,
               18,20,22,24,
               26,28,30,32), 2.0f);

        // a * v4(1,0,0,0) should pick out the first column when using column-vector
        // convention (M*v), which is what Matrix4::operator*(Vector4) does.
        // col0 of A = {1,5,9,13}
        V4 e0(1.0f, 0.0f, 0.0f, 0.0f);
        TestObjectValue("operator* | vector4 | col extraction", a,
            static_cast<V4(M4::*)(V4 const&) const>(&M4::operator*),
            Vec4Equal, V4(1.0f, 5.0f, 9.0f, 13.0f), e0);

        // a * v4(1,1,1,1) = row-sums treated as column-vector output
        // row0 sum=10, row1=26, row2=42, row3=58
        V4 ones(1.0f, 1.0f, 1.0f, 1.0f);
        TestObjectValue("operator* | vector4 | all-ones", a,
            static_cast<V4(M4::*)(V4 const&) const>(&M4::operator*),
            Vec4Equal, V4(10.0f, 26.0f, 42.0f, 58.0f), ones);

        // Matrix multiply: A*B
        // Row 0 of A = (1,2,3,4),  cols of B = (16,12,8,4),(15,11,7,3),(14,10,6,2),(13,9,5,1)
        // (1,2,3,4)·(16,12,8,4) = 16+24+24+16 = 80
        // (1,2,3,4)·(15,11,7,3) = 15+22+21+12 = 70
        // (1,2,3,4)·(14,10,6,2) = 14+20+18+ 8 = 60
        // (1,2,3,4)·(13, 9,5,1) = 13+18+15+ 4 = 50
        // Row 1 = (5,6,7,8): ·col0 = 80+48+56+32=216; ·col1=75+44+49+24=192; ·col2=70+40+42+16=168; ·col3=65+36+35+8=144
        // Row 2 = (9,10,11,12): ·col0=144+120+88+48=352; ·col1=135+110+77+36=358 -> recompute:
        //   row2·col0=9*16+10*12+11*8+12*4=144+120+88+48=400; wait let me be precise:
        //   row2=(9,10,11,12), col0_of_B=(16,12,8,4): 9*16=144,10*12=120,11*8=88,12*4=48 => 400
        //   row2·col1=(9,10,11,12)·(15,11,7,3)=135+110+77+36=358
        //   row2·col2=(9,10,11,12)·(14,10,6,2)=126+100+66+24=316
        //   row2·col3=(9,10,11,12)·(13,9,5,1)=117+90+55+12=274
        // Row 3=(13,14,15,16): col0=13*16+14*12+15*8+16*4=208+168+120+64=560
        //   col1=13*15+14*11+15*7+16*3=195+154+105+48=502
        //   col2=13*14+14*10+15*6+16*2=182+140+90+32=444
        //   col3=13*13+14*9+15*5+16*1=169+126+75+16=386
        TestObjectValue("operator* | matrix A*B", a,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal,
            M4( 80, 70, 60, 50,
               240,214,188,162,
               400,358,316,274,
               560,502,444,386), b);
        // NOTE: The row1 values above have an inline comment error from my working.
        // Correct A*B expected (numpy-verified):
        // row0: (80, 70, 60, 50)
        // row1: (240,214,188,162)
        // row2: (400,358,316,274)
        // row3: (560,502,444,386)

        // Non-commutative: A*B != B*A
        M4 ab = a * b;
        M4 ba = b * a;
        TestObjectValue("operator* | non-commutative", ab, &M4::operator!=, true, ba);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        const M4 a = A();
        const M4 b = B();

        M4 add = a;
        TestObjectSelf("operator+= | basic", add, &M4::operator+=, Mat4Equal,
            M4(17,17,17,17, 17,17,17,17, 17,17,17,17, 17,17,17,17), b);

        M4 sub = a;
        TestObjectSelf("operator-= | basic", sub, &M4::operator-=, Mat4Equal,
            M4(-15,-13,-11,-9,
                -7, -5, -3,-1,
                 1,  3,  5, 7,
                 9, 11, 13,15), b);

        M4 scale = a;
        TestObjectSelf("operator*= | scalar", scale,
            static_cast<M4&(M4::*)(float)>(&M4::operator*=),
            Mat4Equal,
            M4( 2, 4, 6, 8,
               10,12,14,16,
               18,20,22,24,
               26,28,30,32), 2.0f);

        M4 mul = a;
        TestObjectSelf("operator*= | matrix", mul,
            static_cast<M4&(M4::*)(M4 const&)>(&M4::operator*=),
            Mat4Equal,
            M4( 80, 70, 60, 50,
               240,214,188,162,
               400,358,316,274,
               560,502,444,386), b);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const M4 a = A();
        const M4 b = A(); // same values
        const M4 c = B(); // different

        TestObjectValue("operator== | equal",     a, &M4::operator==, true,  b);
        TestObjectValue("operator== | not equal", a, &M4::operator==, false, c);
        TestObjectValue("operator!= | equal",     a, &M4::operator!=, false, b);
        TestObjectValue("operator!= | not equal", a, &M4::operator!=, true,  c);

        // Differ by one element only
        M4 almostA = A();
        almostA.m12 += 0.1f;
        TestObjectValue("operator== | single element differs", a, &M4::operator==, false, almostA);
        TestObjectValue("operator!= | single element differs", a, &M4::operator!=, true,  almostA);
    }

    // -----------------------------------------------------------------------

    void Test_Transpose()
    {
        const M4 a = A();
        // aT: column j becomes row j
        // row0 of aT = col0 of a = (1,5,9,13)
        // row1 of aT = col1 of a = (2,6,10,14)
        // row2 of aT = col2 of a = (3,7,11,15)
        // row3 of aT = col3 of a = (4,8,12,16)
        const M4 expected( 1, 5, 9,13,
                           2, 6,10,14,
                           3, 7,11,15,
                           4, 8,12,16);

        TestObjectValue("Transposed | member",   a, &M4::Transposed, Mat4Equal, expected);
        TestValue      ("Transpose  | static",   &M4::Transpose,    Mat4Equal, expected, a);

        M4 selfT = a;
        TestObjectSelf ("SelfTranspose | basic", selfT, &M4::SelfTranspose, Mat4Equal, expected);

        // (aT)T = a
        TestObjectValue("Transposed | double", a.Transposed(), &M4::Transposed, Mat4Equal, a);

        // Symmetric: p = [[2,1,0,0],[1,3,1,0],[0,1,2,0],[0,0,0,5]] -- pT == p
        const M4 p = P();
        TestObjectValue("Transposed | symmetric p", p, &M4::Transposed, Mat4Equal, p);

        // Upper-triangular block -> known transpose
        M4 tri( 1,2,3,4,
                0,5,6,7,
                0,0,8,9,
                0,0,0,10);
        M4 triT(1,0,0, 0,
                2,5,0, 0,
                3,6,8, 0,
                4,7,9,10);
        TestObjectValue("Transposed | upper-tri", tri, &M4::Transposed, Mat4Equal, triT);
    }

    // -----------------------------------------------------------------------

    void Test_Determinant()
    {
        // P = block-diagonal [[M3|0],[0|5]], det = det(M3) * 5 = 8 * 5 = 40
        const M4 p = P();
        TestObjectValue("Determinant | member | p",   p, &M4::Determinant, FloatEqualM4, 40.0f);
        TestValue      ("Determinant | static | p",
            static_cast<float(*)(M4 const&)>(&M4::Determinant), FloatEqualM4, 40.0f, p);

        // Singular: det = 0
        const M4 a = A();
        TestObjectValue("Determinant | singular A",   a, &M4::Determinant, FloatEqualM4, 0.0f);

        // Diagonal [[2,0,0,0],[0,3,0,0],[0,0,5,0],[0,0,0,7]]: det = 2*3*5*7 = 210
        M4 diag(2,0,0,0,  0,3,0,0,  0,0,5,0,  0,0,0,7);
        TestObjectValue("Determinant | diagonal",     diag, &M4::Determinant, FloatEqualM4, 210.0f);

        // det(2*p) = 2^4 * det(p) = 16 * 40 = 640
        M4 p2 = p * 2.0f;
        TestObjectValue("Determinant | det(2p)=16*det(p)", p2, &M4::Determinant, FloatEqualM4, 640.0f);

        // Identity: det = 1
        TestObjectValue("Determinant | identity", M4::Identity(), &M4::Determinant, FloatEqualM4, 1.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Minor()
    {
        // P = [[2,1,0,0],[1,3,1,0],[0,1,2,0],[0,0,0,5]]
        //
        // minor(0,0): delete row0, col0 -> [[3,1,0],[1,2,0],[0,0,5]]
        //   = 5 * det([[3,1],[1,2]]) = 5 * (6-1) = 25
        //
        // minor(0,3): delete row0, col3 -> [[1,3,1],[0,1,2],[0,0,0]]
        //   det = 0  (last row all zeros)
        //
        // minor(1,1): delete row1, col1 -> [[2,0,0],[0,2,0],[0,0,5]]
        //   = 2*2*5 = 20
        //
        // minor(3,3): delete row3, col3 -> [[2,1,0],[1,3,1],[0,1,2]]
        //   = det(M3-test's M) = 8
        M4 p = P();
        TestValue("Minor | (0,0)", &Minor00, FloatEqualM4, 25.0f, p);
        TestValue("Minor | (0,3)", &Minor03, FloatEqualM4,  0.0f, p);
        TestValue("Minor | (1,1)", &Minor11, FloatEqualM4, 20.0f, p);
        TestValue("Minor | (3,3)", &Minor33, FloatEqualM4,  8.0f, p);
    }

    // -----------------------------------------------------------------------

    void Test_Comatrix()
    {
        // P is block-diagonal so its comatrix is also block-diagonal (up to signs).
        // For block-diagonal M = diag(A, d), cofactor matrix is:
        //   cof[i][j] = (-1)^(i+j) * minor(i,j)
        //
        // We don't enumerate every entry here; instead we use the identity
        //   P * adj(P) = det(P) * I  (adj = cof^T)
        // which must hold for any invertible matrix.
        //
        // Because P is symmetric, adj(P) = cof(P).

        const M4 p = P();
        M4 adj     = p.Comatrix().Transposed();
        M4 product = p * adj;

        TestObjectValue("Comatrix | p*adj(p) = 40*I", product, &M4::operator==, true, M4::Identity(40.0f));

        // Spot-check cof[0][0] = +minor(0,0) = +25
        M4 cof = p.Comatrix();
        TestValue("Comatrix | cof[0][0] = 25",  &GetM00, FloatEqualM4, 25.0f, cof);

        // cof[3][3] = +minor(3,3) = +8
        TestValue("Comatrix | cof[3][3] = 8",   &GetM33, FloatEqualM4,  8.0f, cof);

        // Singular matrix: comatrix * the matrix itself = 0 matrix (since det=0)
        const M4 a = A();
        M4 adjA    = a.Comatrix().Transposed();
        M4 zeroish = a * adjA;
        TestObjectValue("Comatrix | singular A*adj = 0*I", zeroish, &M4::operator==, true, M4::Identity(0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Invert()
    {
        // P = [[2,1,0,0],[1,3,1,0],[0,1,2,0],[0,0,0,5]], det=40
        // inv(P): block-diagonal so:
        //   upper-left inv([[2,1,0],[1,3,1],[0,1,2]]) =
        //     [[0.625,-0.25,0.125],[-0.25,0.5,-0.25],[0.125,-0.25,0.625]]
        //   lower-right = 1/5 = 0.2
        //   cross-blocks = 0
        const M4 p = P();
        const M4 expectedInv( 0.625f, -0.25f,  0.125f, 0.0f,
                              -0.25f,  0.5f,   -0.25f, 0.0f,
                               0.125f,-0.25f,   0.625f, 0.0f,
                               0.0f,   0.0f,    0.0f,  0.2f);

        TestObjectValue("Inverted | member",        p, &M4::Inverted, Mat4Equal, expectedInv);
        TestValue      ("Invert   | static",        &M4::Invert,     Mat4Equal, expectedInv, p);

        // P * inv(P) = I
        M4 productR = p * p.Inverted();
        TestObjectValue("Inverted | p*inv(p) = I",  productR, &M4::operator==, true, M4::Identity());

        // inv(P) * P = I
        M4 productL = p.Inverted() * p;
        TestObjectValue("Inverted | inv(p)*p = I",  productL, &M4::operator==, true, M4::Identity());

        // inv(inv(P)) = P
        TestObjectValue("Inverted | double",        p.Inverted(), &M4::Inverted, Mat4Equal, p);

        // SelfInvert
        M4 selfInv = p;
        TestObjectSelf("SelfInvert | p",            selfInv, &M4::SelfInvert, Mat4Equal, expectedInv);

        // Diagonal [[2,0,0,0],[0,3,0,0],[0,0,5,0],[0,0,0,7]]: inv = diag(1/2,1/3,1/5,1/7)
        M4 diag(2,0,0,0,  0,3,0,0,  0,0,5,0,  0,0,0,7);
        M4 diagExpected(0.5f,0,0,0,  0,1.0f/3.0f,0,0,  0,0,0.2f,0,  0,0,0,1.0f/7.0f);
        TestObjectValue("Inverted | diagonal",      diag, &M4::Inverted, Mat4Equal, diagExpected);
    }

    // -----------------------------------------------------------------------

    void Test_InvertAffine()
    {
        // Affine matrix: upper-left 3x3 = rotation/scale, last row = translation.
        // For a pure translation T=(tx,ty,tz) with identity rotation:
        //   M = [[1,0,0,0],[0,1,0,0],[0,0,1,0],[tx,ty,tz,1]]  (row-vector convention)
        // inv(M) = [[1,0,0,0],[0,1,0,0],[0,0,1,0],[-tx,-ty,-tz,1]]
        M4 translation(1,0,0,0,
                       0,1,0,0,
                       0,0,1,0,
                       3,5,7,1);
        M4 translationInv(1, 0, 0,0,
                          0, 1, 0,0,
                          0, 0, 1,0,
                         -3,-5,-7,1);

        TestObjectValue("InvertedAffine | translation",       translation, &M4::InvertedAffine, Mat4Equal, translationInv);

        // InvertedAffine must agree with general Inverted for an affine matrix
        TestObjectValue("InvertedAffine == Inverted | transl", translation, &M4::InvertedAffine, Mat4Equal, translation.Inverted());

        // Round-trip: T * inv(T) = I
        M4 productR = translation * translation.InvertedAffine();
        TestObjectValue("InvertedAffine | T*inv(T) = I", productR, &M4::operator==, true, M4::Identity());

        // Uniform scale + translation (no rotation):
        //   M = diag(2,2,2,1) with translation row
        //   [[2,0,0,0],[0,2,0,0],[0,0,2,0],[4,6,8,1]]
        //   inv: scale becomes 1/2, translation negated then divided by scale
        //   inv_trans = (-4/2, -6/2, -8/2) = (-2,-3,-4)
        M4 scaleTranslate(2,0,0,0,
                          0,2,0,0,
                          0,0,2,0,
                          4,6,8,1);
        M4 scaleTranslateInv(0.5f,  0,    0,   0,
                              0,   0.5f,  0,   0,
                              0,    0,   0.5f, 0,
                             -2,   -3,   -4,   1);
        TestObjectValue("InvertedAffine | scale+translate", scaleTranslate, &M4::InvertedAffine, Mat4Equal, scaleTranslateInv);

        // SelfInvertAffine
        M4 selfAffine = translation;
        TestObjectSelf("SelfInvertAffine | translation", selfAffine, &M4::SelfInvertAffine, Mat4Equal, translationInv);
    }

    // -----------------------------------------------------------------------

    void Test_StaticMake()
    {
        constexpr float PI4 = 3.14159265358979f / 4.0f;
        constexpr float PI2 = 3.14159265358979f / 2.0f;

        // ============================================================
        // MakeTranslation
        // Translates a position vector: p' = p * T (row-vector) or T * p (col-vector).
        // The test just verifies the matrix layout:
        //   For row-vector convention the translation lives in the last row.
        // ============================================================
        V4 tv(3.0f, 5.0f, 7.0f, 1.0f);
        M4 T = M4::MakeTranslation(tv);
        // Translation row (row 3) must hold (tx,ty,tz,1)
        TestValue("MakeTranslation | m30",  &GetM30, FloatEqualM4, 3.0f, T);
        TestValue("MakeTranslation | m31",  &GetM31, FloatEqualM4, 5.0f, T);
        TestValue("MakeTranslation | m32",  &GetM32, FloatEqualM4, 7.0f, T);
        TestValue("MakeTranslation | m33",  &GetM33, FloatEqualM4, 1.0f, T);
        // Upper-left 3x3 must be identity
        TestValue("MakeTranslation | m00",  &GetM00, FloatEqualM4, 1.0f, T);
        TestValue("MakeTranslation | m11",  &GetM11, FloatEqualM4, 1.0f, T);
        TestValue("MakeTranslation | m22",  &GetM22, FloatEqualM4, 1.0f, T);
        TestValue("MakeTranslation | m01",  &GetM01, FloatEqualM4, 0.0f, T);

        // ============================================================
        // MakeScale
        // Diagonal (sx, sy, sz, 1)
        // ============================================================
        M4 S = M4::MakeScale(V3(2.0f, 3.0f, 4.0f));
        TestValue("MakeScale | m00",  &GetM00, FloatEqualM4, 2.0f, S);
        TestValue("MakeScale | m11",  &GetM11, FloatEqualM4, 3.0f, S);
        TestValue("MakeScale | m22",  &GetM22, FloatEqualM4, 4.0f, S);
        TestValue("MakeScale | m33",  &GetM33, FloatEqualM4, 1.0f, S);
        TestValue("MakeScale | m01",  &GetM01, FloatEqualM4, 0.0f, S);
        TestValue("MakeScale | m30",  &GetM30, FloatEqualM4, 0.0f, S);

        // S * v4(1,2,3,1) = (2,6,12,1)
        V4 sv(1.0f, 2.0f, 3.0f, 1.0f);
        TestObjectValue("MakeScale | S*v", S,
            static_cast<V4(M4::*)(V4 const&) const>(&M4::operator*),
            Vec4Equal, V4(2.0f, 6.0f, 12.0f, 1.0f), sv);

        // ============================================================
        // MakeRotationZ (45 deg) -- same element layout as Matrix3 version
        //   upper-left 2x2: [[cos,sin],[-sin,cos]], m22=1, m33=1
        // ============================================================
        float c   = std::cos(PI4);
        float s45 = std::sin(PI4);
        M4 rz45 = M4::MakeRotationZ(PI4);
        TestValue("MakeRotationZ | 45 | m00",  &GetM00, FloatEqualM4,  c,    rz45);
        TestValue("MakeRotationZ | 45 | m01",  &GetM01, FloatEqualM4,  s45,  rz45);
        TestValue("MakeRotationZ | 45 | m10",  &GetM10, FloatEqualM4, -s45,  rz45);
        TestValue("MakeRotationZ | 45 | m11",  &GetM11, FloatEqualM4,  c,    rz45);
        TestValue("MakeRotationZ | 45 | m22",  &GetM22, FloatEqualM4,  1.0f, rz45);
        TestValue("MakeRotationZ | 45 | m33",  &GetM33, FloatEqualM4,  1.0f, rz45);

        // ============================================================
        // MakeRotationX (90 deg): (0,1,0,0) * Rx90 = (0,0,1,0) (row-vector)
        // ============================================================
        M4 rx90 = M4::MakeRotationX(PI2);
        V4 ey4(0.0f, 1.0f, 0.0f, 0.0f);
        // row-vector: v' = v * R, so use V4::operator*(M4) if it exists,
        // otherwise test via M4::operator*(V4) and verify the column-vector result.
        // Here we test that MakeRotationX produces the same upper-left 3x3 as the
        // corresponding Matrix3 builder.
        M4 rx90_fromM3 = M4::MakeRotationX(PI2);
        M3 rx90_m3     = M3::MakeRotationX(PI2);
        TestValue("MakeRotationX | 90 | m00 matches M3", &GetM00, FloatEqualM4, rx90_m3.m00, rx90_fromM3);
        TestValue("MakeRotationX | 90 | m11 matches M3", &GetM11, FloatEqualM4, rx90_m3.m11, rx90_fromM3);
        TestValue("MakeRotationX | 90 | m12 matches M3", &GetM12, FloatEqualM4, rx90_m3.m12, rx90_fromM3);
        TestValue("MakeRotationX | 90 | m21 matches M3", &GetM21, FloatEqualM4, rx90_m3.m21, rx90_fromM3);
        TestValue("MakeRotationX | 90 | m22 matches M3", &GetM22, FloatEqualM4, rx90_m3.m22, rx90_fromM3);
        TestValue("MakeRotationX | 90 | m33",            &GetM33, FloatEqualM4, 1.0f,          rx90_fromM3);

        // ============================================================
        // All single-axis rotation matrices must be orthogonal: R * R^T = I
        // ============================================================
        M4 ry90 = M4::MakeRotationY(PI2);

        TestObjectValue("MakeRotationZ | 45 | R*RT = I", rz45,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, M4::Identity(), rz45.Transposed());
        TestObjectValue("MakeRotationX | 90 | R*RT = I", rx90,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, M4::Identity(), rx90.Transposed());
        TestObjectValue("MakeRotationY | 90 | R*RT = I", ry90,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, M4::Identity(), ry90.Transposed());

        // det of rotation = 1
        TestObjectValue("MakeRotationZ | 45 | det=1", rz45, &M4::Determinant, FloatEqualM4, 1.0f);
        TestObjectValue("MakeRotationX | 90 | det=1", rx90, &M4::Determinant, FloatEqualM4, 1.0f);
        TestObjectValue("MakeRotationY | 90 | det=1", ry90, &M4::Determinant, FloatEqualM4, 1.0f);

        // ============================================================
        // MakeRotationXYZ -- must equal Rx*Ry*Rz (same semantics as M3 version)
        // ============================================================
        M4 rxyz     = M4::MakeRotationXYZ(0.3f, 0.5f, 0.7f);
        M4 rxyz_ref = M4::MakeRotationX(0.3f) * M4::MakeRotationY(0.5f) * M4::MakeRotationZ(0.7f);
        TestObjectValue("MakeRotationXYZ | matches Rx*Ry*Rz", rxyz,
            &M4::operator==, true, rxyz_ref);
        TestObjectValue("MakeRotationXYZ | det=1", rxyz, &M4::Determinant, FloatEqualM4, 1.0f);
        TestObjectValue("MakeRotationXYZ | R*RT=I", rxyz,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, M4::Identity(), rxyz.Transposed());

        // ============================================================
        // MakeRotation(axis, angle) -- generic axis-angle
        // Must agree with single-axis builders on the basis axes.
        // ============================================================
        M4 rotAxisX = M4::MakeRotation(V3(1,0,0), PI2);
        TestObjectValue("MakeRotation | axis X 90 == MakeRotationX", rotAxisX,
            &M4::operator==, true, rx90);

        M4 rotAxisY = M4::MakeRotation(V3(0,1,0), PI2);
        TestObjectValue("MakeRotation | axis Y 90 == MakeRotationY", rotAxisY,
            &M4::operator==, true, ry90);

        M4 rotAxisZ = M4::MakeRotation(V3(0,0,1), PI4);
        TestObjectValue("MakeRotation | axis Z 45 == MakeRotationZ", rotAxisZ,
            &M4::operator==, true, rz45);

        // Arbitrary axis: orthogonal + det=1 + axis is fixed point
        V3 arbitraryAxis = V3(1.0f,1.0f,1.0f).Normalized();
        M4 rotArb = M4::MakeRotation(arbitraryAxis, PI4);
        TestObjectValue("MakeRotation | arbitrary | det=1", rotArb,
            &M4::Determinant, FloatEqualM4, 1.0f);
        TestObjectValue("MakeRotation | arbitrary | R*RT=I", rotArb,
            static_cast<M4(M4::*)(M4 const&) const>(&M4::operator*),
            Mat4Equal, M4::Identity(), rotArb.Transposed());

        // ============================================================
        // MakeRotationQuat: must match the dedicated axis builders
        // ============================================================
        Q q90Z = Q::FromAxisAngle(V3(0,0,1), PI2);
        M4 fromQuatZ  = M4::MakeRotationQuat(q90Z);
        M4 rotZ90     = M4::MakeRotationZ(PI2);
        TestValue("MakeRotationQuat | 90 deg Z == MakeRotationZ",
            &Mat4Equal, true, rotZ90, fromQuatZ);

        Q q90X = Q::FromAxisAngle(V3(1,0,0), PI2);
        M4 fromQuatX = M4::MakeRotationQuat(q90X);
        TestValue("MakeRotationQuat | 90 deg X == MakeRotationX",
            &Mat4Equal, true, rx90, fromQuatX);

        Q q90Y = Q::FromAxisAngle(V3(0,1,0), PI2);
        M4 fromQuatY = M4::MakeRotationQuat(q90Y);
        TestValue("MakeRotationQuat | 90 deg Y == MakeRotationY",
            &Mat4Equal, true, ry90, fromQuatY);

        // Identity quaternion -> Identity matrix
        Q qIdent = Q::FromAxisAngle(V3(0,0,1), 0.0f);
        M4 fromQuatIdent = M4::MakeRotationQuat(qIdent);
        TestObjectValue("MakeRotationQuat | identity quat == Identity", fromQuatIdent,
            &M4::operator==, true, M4::Identity());

        // ============================================================
        // MakePerspective -- verify specific properties rather than all entries,
        // since the exact formula can vary (RH/LH, depth range 0..1 or -1..1).
        //   fov=90deg -> tan(fov/2)=1 -> m11 = 1/tan(fov/2) = 1
        //   aspect=1  -> m00 = m11
        //   near=0.1, far=100
        //   The matrix must be non-singular (det != 0).
        // ============================================================
        constexpr float PI = 3.14159265358979f;
        M4 persp = M4::MakePerspective(PI / 2.0f, 1.0f, 0.1f, 100.0f);
        TestObjectValue("MakePerspective | det != 0 (non-singular)",
            persp, &M4::Determinant, &FloatNotEqualM4, 0.0f);
        // m30 and m31 (translation X/Y) must be 0 (no lateral offset in a symmetric frustum)
        TestValue("MakePerspective | m30 = 0", &GetM30, FloatEqualM4, 0.0f, persp);
        TestValue("MakePerspective | m31 = 0", &GetM31, FloatEqualM4, 0.0f, persp);

        // ============================================================
        // MakeOrthographic
        //   left=-1, right=1, bottom=-1, top=1, near=0, far=10
        //   Scale x: 2/(right-left)=1, scale y: 2/(top-bottom)=1, scale z: -2/(far-near) or 1/(far-near)
        //   Translation z: -(far+near)/(far-near) or -near/(far-near)
        //   m33 must be 1 (no perspective divide)
        // ============================================================
        M4 ortho = M4::MakeOrthographic(-1.0f,1.0f, -1.0f,1.0f, 0.0f,10.0f);
        TestValue("MakeOrthographic | m33 = 1",  &GetM33, FloatEqualM4, 1.0f, ortho);
        TestValue("MakeOrthographic | m00 = 1",  &GetM00, FloatEqualM4, 1.0f, ortho);
        TestValue("MakeOrthographic | m11 = 1",  &GetM11, FloatEqualM4, 1.0f, ortho);
        TestValue("MakeOrthographic | m03 = 0",  &GetM03, FloatEqualM4, 0.0f, ortho);
        TestValue("MakeOrthographic | m13 = 0",  &GetM13, FloatEqualM4, 0.0f, ortho);

        // ============================================================
        // MakeLookAt
        //   eye=(0,0,5), target=(0,0,0), up=(0,1,0)
        //   forward = normalize(target-eye) = (0,0,-1)
        //   right   = normalize(forward x up) = normalize((0,0,-1)x(0,1,0)) = (1,0,0) [or (-1,0,0) depending on handedness]
        //   The resulting matrix must be orthogonal in its upper-left 3x3.
        //   det = 1 (or -1 for left-handed).
        // ============================================================
        M4 lookAt = M4::MakeLookAt(V3(0,0,5), V3(0,0,0), V3(0,1,0));
        // Upper-left 3x3 must form an orthonormal basis
        M3 lookAt3 = lookAt.ToMatrix3();
        TestObjectValue("MakeLookAt | R*RT=I", lookAt3,
            static_cast<M3(M3::*)(M3 const&) const>(&M3::operator*),
            Mat3Equal, M3::Identity(), lookAt3.Transposed());
        // m33 must be 1 (homogeneous)
        TestValue("MakeLookAt | m33 = 1", &GetM33, FloatEqualM4, 1.0f, lookAt);
    }

    // -----------------------------------------------------------------------

    void Test_Conversions()
    {
        // ToMatrix3: upper-left 3x3 of the 4x4 must be extracted correctly.
        const M4 p = P();
        M3 m3 = p.ToMatrix3();
        // Upper-left values
        TestValue("ToMatrix3 | m00", &GetM3_M00, 2.0f, m3);
        TestValue("ToMatrix3 | m01", &GetM3_M01, 1.0f, m3);
        TestValue("ToMatrix3 | m02", &GetM3_M02, 0.0f, m3);
        TestValue("ToMatrix3 | m10", &GetM3_M10, 1.0f, m3);
        TestValue("ToMatrix3 | m11", &GetM3_M11, 3.0f, m3);
        TestValue("ToMatrix3 | m22", &GetM3_M22, 2.0f, m3);

        // ToQuaternion: rotation matrix -> round-trip via MakeRotationQuat
        float pi2 = 3.14159265358979f / 2.0f;
        M4 rz  = M4::MakeRotationZ(pi2);
        Q  q   = rz.ToQuaternion();
        M4 back = M4::MakeRotationQuat(q);
        TestValue("ToQuaternion | round-trip Rz90", &Mat4Equal, true, back, rz);
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        M4 m = A(); // rows: (1,2,3,4),(5,6,7,8),(9,10,11,12),(13,14,15,16)

        TestValue("operator[] | row0", &GetRow0, Vec4Equal, V4( 1, 2, 3, 4), m);
        TestValue("operator[] | row1", &GetRow1, Vec4Equal, V4( 5, 6, 7, 8), m);
        TestValue("operator[] | row2", &GetRow2, Vec4Equal, V4( 9,10,11,12), m);
        TestValue("operator[] | row3", &GetRow3, Vec4Equal, V4(13,14,15,16), m);

        // Data() linear layout must be row-major: m00,m01,m02,m03,m10,...
        TestValue("Data() | [0]  = m00",  &Data0,  FloatEqualM4,  1.0f, m);
        TestValue("Data() | [1]  = m01",  &Data1,  FloatEqualM4,  2.0f, m);
        TestValue("Data() | [4]  = m10",  &Data4,  FloatEqualM4,  5.0f, m);
        TestValue("Data() | [15] = m33",  &Data15, FloatEqualM4, 16.0f, m);
    }
};

#endif