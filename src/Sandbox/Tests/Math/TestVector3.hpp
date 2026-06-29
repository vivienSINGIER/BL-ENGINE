#ifndef VECTOR3_TEST_H_DEFINED
#define VECTOR3_TEST_H_DEFINED

#include "Test.hpp"
#include "../Render/Common/Math/Vector/Vector3.h"

static bool Vec3Equal(Vector3<float> const& _a, Vector3<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-5f);
}

static bool FloatEqualV3(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-5f);
}

static bool Vec2EqualV3(Vector2<float> const& _a, Vector2<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f);
}

class TestVector3 : public Test
{
public:
    void Run() override
    {
        Test_Constructors();
        Test_Arithmetic();
        Test_CompoundAssignment();
        Test_Comparison();
        Test_Length();
        Test_Normalize();
        Test_DotProduct();
        Test_CrossProduct();
        Test_Reflect();
        Test_Project();
        Test_Perpendicular();
        Test_StaticUtils();
        Test_Swizzle2();
        Test_Swizzle3();
        Test_Subscript();
        DisplayResults();
    }

private:

    using V3 = Vector3<float>;
    using V2 = Vector2<float>;

    static float GetX(V3& _v) { return _v.x; }
    static float GetY(V3& _v) { return _v.y; }
    static float GetZ(V3& _v) { return _v.z; }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        V3 def;
        V3 xyz(1.0f, 2.0f, 3.0f);
        V3 scalar(4.0f);
        V3 xy_z(V2(1.0f, 2.0f), 3.0f);
        V3 x_yz(1.0f, V2(2.0f, 3.0f));
        V3 zero = V3::Zero();
        V3 one  = V3::One();

        TestValue("Constructor | default x",    &GetX, 0.0f, def);
        TestValue("Constructor | default y",    &GetY, 0.0f, def);
        TestValue("Constructor | default z",    &GetZ, 0.0f, def);
        TestValue("Constructor | xyz x",        &GetX, 1.0f, xyz);
        TestValue("Constructor | xyz y",        &GetY, 2.0f, xyz);
        TestValue("Constructor | xyz z",        &GetZ, 3.0f, xyz);
        TestValue("Constructor | scalar x",     &GetX, 4.0f, scalar);
        TestValue("Constructor | scalar y",     &GetY, 4.0f, scalar);
        TestValue("Constructor | scalar z",     &GetZ, 4.0f, scalar);
        TestValue("Constructor | xy_z x",       &GetX, 1.0f, xy_z);
        TestValue("Constructor | xy_z y",       &GetY, 2.0f, xy_z);
        TestValue("Constructor | xy_z z",       &GetZ, 3.0f, xy_z);
        TestValue("Constructor | x_yz x",       &GetX, 1.0f, x_yz);
        TestValue("Constructor | x_yz y",       &GetY, 2.0f, x_yz);
        TestValue("Constructor | x_yz z",       &GetZ, 3.0f, x_yz);
        TestValue("Zero | x",                   &GetX, 0.0f, zero);
        TestValue("Zero | y",                   &GetY, 0.0f, zero);
        TestValue("Zero | z",                   &GetZ, 0.0f, zero);
        TestValue("One | x",                    &GetX, 1.0f, one);
        TestValue("One | y",                    &GetY, 1.0f, one);
        TestValue("One | z",                    &GetZ, 1.0f, one);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const V3 a(3.0f, 4.0f, 5.0f);
        const V3 b(1.0f, 2.0f, 3.0f);

        TestObjectValue("operator+ | basic",        a, &V3::operator+, Vec3Equal, V3(4.0f, 6.0f, 8.0f),  b);
        TestObjectValue("operator- | basic",        a, 
            static_cast<V3(V3::*)(V3 const&) const>(&V3::operator-), 
            Vec3Equal, V3(2.0f, 2.0f, 2.0f),  b);
        TestObjectValue("operator* | basic",        a,
            static_cast<V3(V3::*)(V3 const&) const>(&V3::operator*),
            Vec3Equal, V3(3.0f, 8.0f, 15.0f), b);
        TestObjectValue("operator/ | basic",        a,
            static_cast<V3(V3::*)(V3 const&) const>(&V3::operator/),
            Vec3Equal, V3(3.0f, 2.0f, 5.0f/3.0f), b);
        TestObjectValue("operator* | scalar",       a,
            static_cast<V3(V3::*)(float) const>(&V3::operator*),
            Vec3Equal, V3(6.0f, 8.0f, 10.0f), 2.0f);
        TestObjectValue("operator/ | scalar",       a,
            static_cast<V3(V3::*)(float) const>(&V3::operator/),
            Vec3Equal, V3(1.5f, 2.0f, 2.5f), 2.0f);

        TestValue("operator* | scalar reverse",
            static_cast<V3(*)(float, V3 const&)>(&operator*),
            Vec3Equal, V3(6.0f, 8.0f, 10.0f), 2.0f, a);

        // zero / negative
        const V3 zero(0.0f);
        const V3 neg(-1.0f, -2.0f, -3.0f);
        TestObjectValue("operator+ | zero",         a, &V3::operator+, Vec3Equal, a,                      zero);
        TestObjectValue("operator- | self",         a, 
            static_cast<V3(V3::*)(V3 const&) const>(&V3::operator-), 
            Vec3Equal, V3(0.0f),               a);
        TestObjectValue("operator+ | negative",     a, &V3::operator+, Vec3Equal, V3(2.0f, 2.0f, 2.0f),  neg);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        V3 a(3.0f, 4.0f, 5.0f);
        V3 b(1.0f, 2.0f, 3.0f);

        TestObjectSelf("operator+= | basic",    a, &V3::operator+=, Vec3Equal, V3(4.0f, 6.0f,  8.0f),      b);
        TestObjectSelf("operator-= | basic",    a, &V3::operator-=, Vec3Equal, V3(2.0f, 2.0f,  2.0f),      b);
        TestObjectSelf("operator*= | basic",    a,
            static_cast<V3&(V3::*)(V3 const&)>(&V3::operator*=),
            Vec3Equal, V3(3.0f, 8.0f, 15.0f), b);
        TestObjectSelf("operator/= | basic",    a,
            static_cast<V3&(V3::*)(V3 const&)>(&V3::operator/=),
            Vec3Equal, V3(3.0f, 2.0f, 5.0f/3.0f), b);
        TestObjectSelf("operator*= | scalar",   a,
            static_cast<V3&(V3::*)(float)>(&V3::operator*=),
            Vec3Equal, V3(6.0f, 8.0f, 10.0f), 2.0f);
        TestObjectSelf("operator/= | scalar",   a,
            static_cast<V3&(V3::*)(float)>(&V3::operator/=),
            Vec3Equal, V3(1.5f, 2.0f, 2.5f), 2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const V3 a(1.0f, 2.0f, 3.0f);
        const V3 b(1.0f, 2.0f, 3.0f);
        const V3 c(4.0f, 5.0f, 6.0f);

        TestObjectValue("operator== | equal",       a, &V3::operator==, true,  b);
        TestObjectValue("operator== | not equal",   a, &V3::operator==, false, c);
        TestObjectValue("operator!= | equal",       a, &V3::operator!=, false, b);
        TestObjectValue("operator!= | not equal",   a, &V3::operator!=, true,  c);

        V3 zero(0.0f);
        TestObjectValue("IsNull | zero",            zero, &V3::IsNull, true);
        TestObjectValue("IsNull | non-zero",        a,    &V3::IsNull, false);
    }

    // -----------------------------------------------------------------------

    void Test_Length()
    {
        const V3 a(1.0f, 2.0f, 2.0f);   // length = 3
        const V3 unit(1.0f, 0.0f, 0.0f);
        const V3 zero(0.0f);

        TestObjectValue("Length | 1,2,2",           a,    &V3::Length,        FloatEqualV3, 3.0f);
        TestObjectValue("Length | unit",            unit, &V3::Length,        FloatEqualV3, 1.0f);
        TestObjectValue("Length | zero",            zero, &V3::Length,        FloatEqualV3, 0.0f);
        TestObjectValue("LengthSquared | 1,2,2",    a,    &V3::LengthSquared, FloatEqualV3, 9.0f);
        TestObjectValue("LengthSquared | unit",     unit, &V3::LengthSquared, FloatEqualV3, 1.0f);
        TestObjectValue("LengthSquared | zero",     zero, &V3::LengthSquared, FloatEqualV3, 0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Normalize()
    {
        V3 a(1.0f, 2.0f, 2.0f);         // normalized = (1/3, 2/3, 2/3)
        V3 unit(1.0f, 0.0f, 0.0f);

        TestObjectValue("Normalized | 1,2,2",       a,    &V3::Normalized,    Vec3Equal, V3(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f));
        TestObjectValue("Normalized | unit",        unit, &V3::Normalized,    Vec3Equal, V3(1.0f, 0.0f, 0.0f));
        TestObjectSelf ("SelfNormalize | 1,2,2",    a,    &V3::SelfNormalize, Vec3Equal, V3(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f));
        TestObjectSelf ("SelfNormalize | unit",     unit, &V3::SelfNormalize, Vec3Equal, V3(1.0f, 0.0f, 0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DotProduct()
    {
        const V3 a(1.0f, 2.0f, 3.0f);
        const V3 b(4.0f, 5.0f, 6.0f);
        const V3 zero(0.0f);

        // dot(a, perp) where perp is perpendicular to a
        // (1,2,3) . (-2,1,0) = -2+2+0 = 0
        const V3 perp(-2.0f, 1.0f, 0.0f);

        TestValue("Dot | basic",            &V3::Dot, FloatEqualV3, 32.0f, a, b);
        TestValue("Dot | perpendicular",    &V3::Dot, FloatEqualV3,  0.0f, a, perp);
        TestValue("Dot | zero",             &V3::Dot, FloatEqualV3,  0.0f, a, zero);
        TestValue("Dot | self",             &V3::Dot, FloatEqualV3, 14.0f, a, a);
    }

    // -----------------------------------------------------------------------

    void Test_CrossProduct()
    {
        const V3 x(1.0f, 0.0f, 0.0f);
        const V3 y(0.0f, 1.0f, 0.0f);
        const V3 z(0.0f, 0.0f, 1.0f);

        // x ^ y = z,  y ^ x = -z,  x ^ x = 0
        TestObjectValue("operator^ | x cross y",    x, &V3::operator^, Vec3Equal, z,         y);
        TestObjectValue("operator^ | y cross x",    y, &V3::operator^, Vec3Equal, V3(0.0f, 0.0f, -1.0f), x);
        TestObjectValue("operator^ | self",         x, &V3::operator^, Vec3Equal, V3(0.0f),  x);

        // cross product is perpendicular to both inputs — verify via dot
        const V3 a(1.0f, 2.0f, 3.0f);
        const V3 b(4.0f, 5.0f, 6.0f);
        V3 cross = a ^ b;
        TestValue("operator^ | perp to a",  &V3::Dot, FloatEqualV3, 0.0f, a, cross);
        TestValue("operator^ | perp to b",  &V3::Dot, FloatEqualV3, 0.0f, b, cross);

        // compound
        V3 c(1.0f, 0.0f, 0.0f);
        TestObjectSelf("operator^= | x cross y", c, &V3::operator^=, Vec3Equal, z, y);
    }

    // -----------------------------------------------------------------------

    void Test_Reflect()
    {
        const V3 v(1.0f, -1.0f, 0.0f);
        const V3 normalUp  (0.0f, 1.0f, 0.0f);
        const V3 normalRight(1.0f, 0.0f, 0.0f);

        TestObjectValue("Reflect | floor normal",   v, &V3::Reflect, Vec3Equal, V3( 1.0f, 1.0f, 0.0f), normalUp);
        TestObjectValue("Reflect | wall normal",    v, &V3::Reflect, Vec3Equal, V3(-1.0f,-1.0f, 0.0f), normalRight);
    }

    // -----------------------------------------------------------------------

    void Test_Project()
    {
        const V3 a(3.0f, 4.0f, 5.0f);
        const V3 xAxis(1.0f, 0.0f, 0.0f);
        const V3 yAxis(0.0f, 1.0f, 0.0f);
        const V3 zAxis(0.0f, 0.0f, 1.0f);

        TestObjectValue("Project | onto x axis",    a, &V3::Project, Vec3Equal, V3(3.0f, 0.0f, 0.0f), xAxis);
        TestObjectValue("Project | onto y axis",    a, &V3::Project, Vec3Equal, V3(0.0f, 4.0f, 0.0f), yAxis);
        TestObjectValue("Project | onto z axis",    a, &V3::Project, Vec3Equal, V3(0.0f, 0.0f, 5.0f), zAxis);
        TestObjectValue("Project | onto self",      a, &V3::Project, Vec3Equal, a,                     a);
    }

    // -----------------------------------------------------------------------

    void Test_Perpendicular()
    {
        const V3 a(1.0f, 2.0f, 3.0f);

        // result should be perpendicular to _v (the argument), verify via dot
        V3 perp = a.Perpendicular();
        TestValue("Perpendicular | perp to arg",    &V3::Dot, FloatEqualV3, 0.0f, perp, a);
    }

    // -----------------------------------------------------------------------

    void Test_StaticUtils()
    {
        const V3 a( 2.0f, -3.0f,  1.0f);
        const V3 b(-1.0f,  4.0f, -2.0f);

        TestValue("Min | basic",    &V3::Min,   Vec3Equal, V3(-1.0f, -3.0f, -2.0f), a, b);
        TestValue("Max | basic",    &V3::Max,   Vec3Equal, V3( 2.0f,  4.0f,  1.0f), a, b);
        TestValue("Abs | mixed",    &V3::Abs,   Vec3Equal, V3( 2.0f,  3.0f,  1.0f), a);

        const V3 v( 3.0f, -1.0f,  5.0f);
        const V3 lo(0.0f,  0.0f,  0.0f);
        const V3 hi(2.0f,  2.0f,  2.0f);
        TestValue("Clamp | basic",  &V3::Clamp, Vec3Equal, V3(2.0f, 0.0f, 2.0f), v, lo, hi);

        TestValue("Normalize | 1,2,2", &V3::Normalize, Vec3Equal,
            V3(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f),
            V3(1.0f, 2.0f, 2.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle2()
    {
        const V3 v(1.0f, 2.0f, 3.0f);

        TestObjectValue("xy", v, &V3::xy, Vec2EqualV3, V2(1.0f, 2.0f));
        TestObjectValue("yx", v, &V3::yx, Vec2EqualV3, V2(2.0f, 1.0f));
        TestObjectValue("xz", v, &V3::xz, Vec2EqualV3, V2(1.0f, 3.0f));
        TestObjectValue("zx", v, &V3::zx, Vec2EqualV3, V2(3.0f, 1.0f));
        TestObjectValue("yz", v, &V3::yz, Vec2EqualV3, V2(2.0f, 3.0f));
        TestObjectValue("zy", v, &V3::zy, Vec2EqualV3, V2(3.0f, 2.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle3()
    {
        const V3 v(1.0f, 2.0f, 3.0f);

        TestObjectValue("xzy", v, &V3::xzy, Vec3Equal, V3(1.0f, 3.0f, 2.0f));
        TestObjectValue("yxz", v, &V3::yxz, Vec3Equal, V3(2.0f, 1.0f, 3.0f));
        TestObjectValue("yzx", v, &V3::yzx, Vec3Equal, V3(2.0f, 3.0f, 1.0f));
        TestObjectValue("zxy", v, &V3::zxy, Vec3Equal, V3(3.0f, 1.0f, 2.0f));
        TestObjectValue("zyx", v, &V3::zyx, Vec3Equal, V3(3.0f, 2.0f, 1.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        V3 v(3.0f, 7.0f, 11.0f);
        TestValue("operator[] | x", &GetX, 3.0f,  v);
        TestValue("operator[] | y", &GetY, 7.0f,  v);
        TestValue("operator[] | z", &GetZ, 11.0f, v);
    }
};

#endif