#ifndef VECTOR4_TEST_H_DEFINED
#define VECTOR4_TEST_H_DEFINED

#include "Test.hpp"
#include "../Render/Common/Math/Vector4.h"

static bool Vec4Equal(Vector4<float> const& _a, Vector4<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-5f) &&
           MathUtils::NearlyEqual(_a.w, _b.w, 1e-5f);
}

static bool FloatEqualV4(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-5f);
}

static bool Vec2EqualV4(Vector2<float> const& _a, Vector2<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f);
}

static bool Vec3EqualV4(Vector3<float> const& _a, Vector3<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-5f);
}

class TestVector4 : public Test
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
        Test_StaticUtils();
        Test_Swizzle2();
        Test_Swizzle3();
        Test_Swizzle4();
        Test_Subscript();
        DisplayResults();
    }

private:

    using V4 = Vector4<float>;
    using V3 = Vector3<float>;
    using V2 = Vector2<float>;

    static float GetX(V4& _v) { return _v.x; }
    static float GetY(V4& _v) { return _v.y; }
    static float GetZ(V4& _v) { return _v.z; }
    static float GetW(V4& _v) { return _v.w; }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        V4 def;
        V4 scalar(2.0f);
        V4 xyzw(1.0f, 2.0f, 3.0f, 4.0f);
        V4 v2v2(V2(1.0f, 2.0f), V2(3.0f, 4.0f));
        V4 v3w(V3(1.0f, 2.0f, 3.0f), 4.0f);
        V4 xv3(1.0f, V3(2.0f, 3.0f, 4.0f));
        V4 v2zw(V2(1.0f, 2.0f), 3.0f, 4.0f);
        V4 xv2w(1.0f, V2(2.0f, 3.0f), 4.0f);
        V4 xyv2(1.0f, 2.0f, V2(3.0f, 4.0f));
        V4 zero = V4::Zero();
        V4 one  = V4::One();

        TestValue("Constructor | default x",        &GetX, 0.0f, def);
        TestValue("Constructor | default y",        &GetY, 0.0f, def);
        TestValue("Constructor | default z",        &GetZ, 0.0f, def);
        TestValue("Constructor | default w",        &GetW, 0.0f, def);

        TestValue("Constructor | scalar x",         &GetX, 2.0f, scalar);
        TestValue("Constructor | scalar y",         &GetY, 2.0f, scalar);
        TestValue("Constructor | scalar z",         &GetZ, 2.0f, scalar);
        TestValue("Constructor | scalar w",         &GetW, 2.0f, scalar);

        TestValue("Constructor | xyzw x",           &GetX, 1.0f, xyzw);
        TestValue("Constructor | xyzw y",           &GetY, 2.0f, xyzw);
        TestValue("Constructor | xyzw z",           &GetZ, 3.0f, xyzw);
        TestValue("Constructor | xyzw w",           &GetW, 4.0f, xyzw);

        TestValue("Constructor | v2v2 x",           &GetX, 1.0f, v2v2);
        TestValue("Constructor | v2v2 y",           &GetY, 2.0f, v2v2);
        TestValue("Constructor | v2v2 z",           &GetZ, 3.0f, v2v2);
        TestValue("Constructor | v2v2 w",           &GetW, 4.0f, v2v2);

        TestValue("Constructor | v3w x",            &GetX, 1.0f, v3w);
        TestValue("Constructor | v3w y",            &GetY, 2.0f, v3w);
        TestValue("Constructor | v3w z",            &GetZ, 3.0f, v3w);
        TestValue("Constructor | v3w w",            &GetW, 4.0f, v3w);

        TestValue("Constructor | xv3 x",            &GetX, 1.0f, xv3);
        TestValue("Constructor | xv3 y",            &GetY, 2.0f, xv3);
        TestValue("Constructor | xv3 z",            &GetZ, 3.0f, xv3);
        TestValue("Constructor | xv3 w",            &GetW, 4.0f, xv3);

        TestValue("Constructor | v2zw x",           &GetX, 1.0f, v2zw);
        TestValue("Constructor | v2zw y",           &GetY, 2.0f, v2zw);
        TestValue("Constructor | v2zw z",           &GetZ, 3.0f, v2zw);
        TestValue("Constructor | v2zw w",           &GetW, 4.0f, v2zw);

        TestValue("Constructor | xv2w x",           &GetX, 1.0f, xv2w);
        TestValue("Constructor | xv2w y",           &GetY, 2.0f, xv2w);
        TestValue("Constructor | xv2w z",           &GetZ, 3.0f, xv2w);
        TestValue("Constructor | xv2w w",           &GetW, 4.0f, xv2w);

        TestValue("Constructor | xyv2 x",           &GetX, 1.0f, xyv2);
        TestValue("Constructor | xyv2 y",           &GetY, 2.0f, xyv2);
        TestValue("Constructor | xyv2 z",           &GetZ, 3.0f, xyv2);
        TestValue("Constructor | xyv2 w",           &GetW, 4.0f, xyv2);

        TestValue("Zero | x",                       &GetX, 0.0f, zero);
        TestValue("Zero | y",                       &GetY, 0.0f, zero);
        TestValue("Zero | z",                       &GetZ, 0.0f, zero);
        TestValue("Zero | w",                       &GetW, 0.0f, zero);

        TestValue("One | x",                        &GetX, 1.0f, one);
        TestValue("One | y",                        &GetY, 1.0f, one);
        TestValue("One | z",                        &GetZ, 1.0f, one);
        TestValue("One | w",                        &GetW, 1.0f, one);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const V4 a(2.0f, 4.0f, 6.0f, 8.0f);
        const V4 b(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectValue("operator+ | basic",    a, &V4::operator+, Vec4Equal, V4(3.0f, 6.0f, 9.0f, 12.0f), b);
        TestObjectValue("operator- | basic",    a, &V4::operator-, Vec4Equal, V4(1.0f, 2.0f, 3.0f,  4.0f), b);
        TestObjectValue("operator* | basic",    a,
            static_cast<V4(V4::*)(V4 const&) const>(&V4::operator*),
            Vec4Equal, V4(2.0f, 8.0f, 18.0f, 32.0f), b);
        TestObjectValue("operator/ | basic",    a,
            static_cast<V4(V4::*)(V4 const&) const>(&V4::operator/),
            Vec4Equal, V4(2.0f, 2.0f,  2.0f,  2.0f), b);
        TestObjectValue("operator* | scalar",   a,
            static_cast<V4(V4::*)(float) const>(&V4::operator*),
            Vec4Equal, V4(4.0f, 8.0f, 12.0f, 16.0f), 2.0f);
        TestObjectValue("operator/ | scalar",   a,
            static_cast<V4(V4::*)(float) const>(&V4::operator/),
            Vec4Equal, V4(1.0f, 2.0f,  3.0f,  4.0f), 2.0f);

        TestValue("operator* | scalar reverse",
            static_cast<V4(*)(float, V4 const&)>(&operator*),
            Vec4Equal, V4(4.0f, 8.0f, 12.0f, 16.0f), 2.0f, a);

        const V4 zero(0.0f);
        const V4 neg(-1.0f, -2.0f, -3.0f, -4.0f);
        TestObjectValue("operator+ | zero",     a, &V4::operator+, Vec4Equal, a,                           zero);
        TestObjectValue("operator- | self",     a, &V4::operator-, Vec4Equal, V4(0.0f),                    a);
        TestObjectValue("operator+ | negative", a, &V4::operator+, Vec4Equal, V4(1.0f, 2.0f, 3.0f, 4.0f), neg);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        V4 a(2.0f, 4.0f, 6.0f, 8.0f);
        V4 b(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectSelf("operator+= | basic",    a, &V4::operator+=, Vec4Equal, V4(3.0f,  6.0f,  9.0f, 12.0f), b);
        TestObjectSelf("operator-= | basic",    a, &V4::operator-=, Vec4Equal, V4(1.0f,  2.0f,  3.0f,  4.0f), b);
        TestObjectSelf("operator*= | basic",    a,
            static_cast<V4&(V4::*)(V4 const&)>(&V4::operator*=),
            Vec4Equal, V4(2.0f, 8.0f, 18.0f, 32.0f), b);
        TestObjectSelf("operator/= | basic",    a,
            static_cast<V4&(V4::*)(V4 const&)>(&V4::operator/=),
            Vec4Equal, V4(2.0f,  2.0f,  2.0f,  2.0f), b);
        TestObjectSelf("operator*= | scalar",   a,
            static_cast<V4&(V4::*)(float)>(&V4::operator*=),
            Vec4Equal, V4(4.0f,  8.0f, 12.0f, 16.0f), 2.0f);
        TestObjectSelf("operator/= | scalar",   a,
            static_cast<V4&(V4::*)(float)>(&V4::operator/=),
            Vec4Equal, V4(1.0f,  2.0f,  3.0f,  4.0f), 2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const V4 a(1.0f, 2.0f, 3.0f, 4.0f);
        const V4 b(1.0f, 2.0f, 3.0f, 4.0f);
        const V4 c(5.0f, 6.0f, 7.0f, 8.0f);

        TestObjectValue("operator== | equal",       a, &V4::operator==, true,  b);
        TestObjectValue("operator== | not equal",   a, &V4::operator==, false, c);
        TestObjectValue("operator!= | equal",       a, &V4::operator!=, false, b);
        TestObjectValue("operator!= | not equal",   a, &V4::operator!=, true,  c);

        V4 zero(0.0f);
        TestObjectValue("IsNull | zero",            zero, &V4::IsNull, true);
        TestObjectValue("IsNull | non-zero",        a,    &V4::IsNull, false);
    }

    // -----------------------------------------------------------------------

    void Test_Length()
    {
        // (1,2,2,0) -> length = 3
        const V4 a(1.0f, 2.0f, 2.0f, 0.0f);
        const V4 unit(1.0f, 0.0f, 0.0f, 0.0f);
        const V4 zero(0.0f);

        TestObjectValue("Length | 1,2,2,0",         a,    &V4::Length,        FloatEqualV4, 3.0f);
        TestObjectValue("Length | unit",            unit, &V4::Length,        FloatEqualV4, 1.0f);
        TestObjectValue("Length | zero",            zero, &V4::Length,        FloatEqualV4, 0.0f);
        TestObjectValue("LengthSquared | 1,2,2,0",  a,    &V4::LengthSquared, FloatEqualV4, 9.0f);
        TestObjectValue("LengthSquared | unit",     unit, &V4::LengthSquared, FloatEqualV4, 1.0f);
        TestObjectValue("LengthSquared | zero",     zero, &V4::LengthSquared, FloatEqualV4, 0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Normalize()
    {
        V4 a(1.0f, 2.0f, 2.0f, 0.0f);
        V4 unit(1.0f, 0.0f, 0.0f, 0.0f);

        TestObjectValue("Normalized | 1,2,2,0",     a,    &V4::Normalized,    Vec4Equal, V4(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f, 0.0f));
        TestObjectValue("Normalized | unit",        unit, &V4::Normalized,    Vec4Equal, V4(1.0f, 0.0f, 0.0f, 0.0f));
        TestObjectSelf ("SelfNormalize | 1,2,2,0",  a,    &V4::SelfNormalize, Vec4Equal, V4(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f, 0.0f));
        TestObjectSelf ("SelfNormalize | unit",     unit, &V4::SelfNormalize, Vec4Equal, V4(1.0f, 0.0f, 0.0f, 0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DotProduct()
    {
        const V4 a(1.0f, 2.0f, 3.0f, 4.0f);
        const V4 b(5.0f, 6.0f, 7.0f, 8.0f);
        const V4 zero(0.0f);
        // (1,2,3,4).(-2,1,0,0) = -2+2+0+0 = 0
        const V4 perp(-2.0f, 1.0f, 0.0f, 0.0f);

        TestValue("Dot | basic",            &V4::Dot, FloatEqualV4, 70.0f, a, b);
        TestValue("Dot | perpendicular",    &V4::Dot, FloatEqualV4,  0.0f, a, perp);
        TestValue("Dot | zero",             &V4::Dot, FloatEqualV4,  0.0f, a, zero);
        TestValue("Dot | self",             &V4::Dot, FloatEqualV4, 30.0f, a, a);
    }

    // -----------------------------------------------------------------------

    void Test_StaticUtils()
    {
        const V4 a( 2.0f, -3.0f,  1.0f, -4.0f);
        const V4 b(-1.0f,  4.0f, -2.0f,  5.0f);

        TestValue("Min | basic",    &V4::Min,   Vec4Equal, V4(-1.0f, -3.0f, -2.0f, -4.0f), a, b);
        TestValue("Max | basic",    &V4::Max,   Vec4Equal, V4( 2.0f,  4.0f,  1.0f,  5.0f), a, b);
        TestValue("Abs | mixed",    &V4::Abs,   Vec4Equal, V4( 2.0f,  3.0f,  1.0f,  4.0f), a);

        const V4 v( 3.0f, -1.0f,  5.0f, -2.0f);
        const V4 lo(0.0f,  0.0f,  0.0f,  0.0f);
        const V4 hi(2.0f,  2.0f,  2.0f,  2.0f);
        TestValue("Clamp | basic",  &V4::Clamp, Vec4Equal, V4(2.0f, 0.0f, 2.0f, 0.0f), v, lo, hi);

        TestValue("Normalize | 1,2,2,0", &V4::Normalize, Vec4Equal,
            V4(1.0f/3.0f, 2.0f/3.0f, 2.0f/3.0f, 0.0f),
            V4(1.0f, 2.0f, 2.0f, 0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle2()
    {
        const V4 v(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectValue("xy", v, &V4::xy, Vec2EqualV4, V2(1.0f, 2.0f));
        TestObjectValue("yx", v, &V4::yx, Vec2EqualV4, V2(2.0f, 1.0f));
        TestObjectValue("xz", v, &V4::xz, Vec2EqualV4, V2(1.0f, 3.0f));
        TestObjectValue("zx", v, &V4::zx, Vec2EqualV4, V2(3.0f, 1.0f));
        TestObjectValue("xw", v, &V4::xw, Vec2EqualV4, V2(1.0f, 4.0f));
        TestObjectValue("wx", v, &V4::wx, Vec2EqualV4, V2(4.0f, 1.0f));
        TestObjectValue("yz", v, &V4::yz, Vec2EqualV4, V2(2.0f, 3.0f));
        TestObjectValue("zy", v, &V4::zy, Vec2EqualV4, V2(3.0f, 2.0f));
        TestObjectValue("yw", v, &V4::yw, Vec2EqualV4, V2(2.0f, 4.0f));
        TestObjectValue("wy", v, &V4::wy, Vec2EqualV4, V2(4.0f, 2.0f));
        TestObjectValue("zw", v, &V4::zw, Vec2EqualV4, V2(3.0f, 4.0f));
        TestObjectValue("wz", v, &V4::wz, Vec2EqualV4, V2(4.0f, 3.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle3()
    {
        const V4 v(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectValue("xyz", v, &V4::xyz, Vec3EqualV4, V3(1.0f, 2.0f, 3.0f));
        TestObjectValue("xyw", v, &V4::xyw, Vec3EqualV4, V3(1.0f, 2.0f, 4.0f));
        TestObjectValue("xzy", v, &V4::xzy, Vec3EqualV4, V3(1.0f, 3.0f, 2.0f));
        TestObjectValue("xzw", v, &V4::xzw, Vec3EqualV4, V3(1.0f, 3.0f, 4.0f));
        TestObjectValue("xwy", v, &V4::xwy, Vec3EqualV4, V3(1.0f, 4.0f, 2.0f));
        TestObjectValue("xwz", v, &V4::xwz, Vec3EqualV4, V3(1.0f, 4.0f, 3.0f));
        TestObjectValue("yxz", v, &V4::yxz, Vec3EqualV4, V3(2.0f, 1.0f, 3.0f));
        TestObjectValue("yxw", v, &V4::yxw, Vec3EqualV4, V3(2.0f, 1.0f, 4.0f));
        TestObjectValue("yzx", v, &V4::yzx, Vec3EqualV4, V3(2.0f, 3.0f, 1.0f));
        TestObjectValue("yzw", v, &V4::yzw, Vec3EqualV4, V3(2.0f, 3.0f, 4.0f));
        TestObjectValue("ywx", v, &V4::ywx, Vec3EqualV4, V3(2.0f, 4.0f, 1.0f));
        TestObjectValue("ywz", v, &V4::ywz, Vec3EqualV4, V3(2.0f, 4.0f, 3.0f));
        TestObjectValue("zxy", v, &V4::zxy, Vec3EqualV4, V3(3.0f, 1.0f, 2.0f));
        TestObjectValue("zxw", v, &V4::zxw, Vec3EqualV4, V3(3.0f, 1.0f, 4.0f));
        TestObjectValue("zyx", v, &V4::zyx, Vec3EqualV4, V3(3.0f, 2.0f, 1.0f));
        TestObjectValue("zyw", v, &V4::zyw, Vec3EqualV4, V3(3.0f, 2.0f, 4.0f));
        TestObjectValue("zwx", v, &V4::zwx, Vec3EqualV4, V3(3.0f, 4.0f, 1.0f));
        TestObjectValue("zwy", v, &V4::zwy, Vec3EqualV4, V3(3.0f, 4.0f, 2.0f));
        TestObjectValue("wxy", v, &V4::wxy, Vec3EqualV4, V3(4.0f, 1.0f, 2.0f));
        TestObjectValue("wxz", v, &V4::wxz, Vec3EqualV4, V3(4.0f, 1.0f, 3.0f));
        TestObjectValue("wyx", v, &V4::wyx, Vec3EqualV4, V3(4.0f, 2.0f, 1.0f));
        TestObjectValue("wyz", v, &V4::wyz, Vec3EqualV4, V3(4.0f, 2.0f, 3.0f));
        TestObjectValue("wzx", v, &V4::wzx, Vec3EqualV4, V3(4.0f, 3.0f, 1.0f));
        TestObjectValue("wzy", v, &V4::wzy, Vec3EqualV4, V3(4.0f, 3.0f, 2.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle4()
    {
        const V4 v(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectValue("xywz", v, &V4::xywz, Vec4Equal, V4(1.0f, 2.0f, 4.0f, 3.0f));
        TestObjectValue("xzyw", v, &V4::xzyw, Vec4Equal, V4(1.0f, 3.0f, 2.0f, 4.0f));
        TestObjectValue("xzwy", v, &V4::xzwy, Vec4Equal, V4(1.0f, 3.0f, 4.0f, 2.0f));
        TestObjectValue("xwyz", v, &V4::xwyz, Vec4Equal, V4(1.0f, 4.0f, 2.0f, 3.0f));
        TestObjectValue("xwzy", v, &V4::xwzy, Vec4Equal, V4(1.0f, 4.0f, 3.0f, 2.0f));
        TestObjectValue("yxzw", v, &V4::yxzw, Vec4Equal, V4(2.0f, 1.0f, 3.0f, 4.0f));
        TestObjectValue("yxwz", v, &V4::yxwz, Vec4Equal, V4(2.0f, 1.0f, 4.0f, 3.0f));
        TestObjectValue("yzxw", v, &V4::yzxw, Vec4Equal, V4(2.0f, 3.0f, 1.0f, 4.0f));
        TestObjectValue("yzwx", v, &V4::yzwx, Vec4Equal, V4(2.0f, 3.0f, 4.0f, 1.0f));
        TestObjectValue("ywxz", v, &V4::ywxz, Vec4Equal, V4(2.0f, 4.0f, 1.0f, 3.0f));
        TestObjectValue("ywzx", v, &V4::ywzx, Vec4Equal, V4(2.0f, 4.0f, 3.0f, 1.0f));
        TestObjectValue("zxyw", v, &V4::zxyw, Vec4Equal, V4(3.0f, 1.0f, 2.0f, 4.0f));
        TestObjectValue("zxwy", v, &V4::zxwy, Vec4Equal, V4(3.0f, 1.0f, 4.0f, 2.0f));
        TestObjectValue("zyxw", v, &V4::zyxw, Vec4Equal, V4(3.0f, 2.0f, 1.0f, 4.0f));
        TestObjectValue("zywx", v, &V4::zywx, Vec4Equal, V4(3.0f, 2.0f, 4.0f, 1.0f));
        TestObjectValue("zwxy", v, &V4::zwxy, Vec4Equal, V4(3.0f, 4.0f, 1.0f, 2.0f));
        TestObjectValue("zwyx", v, &V4::zwyx, Vec4Equal, V4(3.0f, 4.0f, 2.0f, 1.0f));
        TestObjectValue("wxyz", v, &V4::wxyz, Vec4Equal, V4(4.0f, 1.0f, 2.0f, 3.0f));
        TestObjectValue("wxzy", v, &V4::wxzy, Vec4Equal, V4(4.0f, 1.0f, 3.0f, 2.0f));
        TestObjectValue("wyxz", v, &V4::wyxz, Vec4Equal, V4(4.0f, 2.0f, 1.0f, 3.0f));
        TestObjectValue("wyzx", v, &V4::wyzx, Vec4Equal, V4(4.0f, 2.0f, 3.0f, 1.0f));
        TestObjectValue("wzxy", v, &V4::wzxy, Vec4Equal, V4(4.0f, 3.0f, 1.0f, 2.0f));
        TestObjectValue("wzyx", v, &V4::wzyx, Vec4Equal, V4(4.0f, 3.0f, 2.0f, 1.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        V4 v(3.0f, 7.0f, 11.0f, 15.0f);
        TestValue("operator[] | x", &GetX,  3.0f, v);
        TestValue("operator[] | y", &GetY,  7.0f, v);
        TestValue("operator[] | z", &GetZ, 11.0f, v);
        TestValue("operator[] | w", &GetW, 15.0f, v);
    }
};

#endif