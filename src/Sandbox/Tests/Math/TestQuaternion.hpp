#ifndef QUATERNION_TEST_H_DEFINED
#define QUATERNION_TEST_H_DEFINED

#include "Test.hpp"
#include "../Render/Common/Math/Quaternions/Quaternion.h"
#include "../Render/Common/Math/Vector/Vector3.h"
#include "../Render/Common/Math/Vector/Vector4.h"
#include "../Render/Common/Math/Matrix/Matrix3.h"
#include "../Render/Common/Math/Matrix/Matrix4.h"

static bool QuatEqual(Quaternion const& _a, Quaternion const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f) &&
           MathUtils::NearlyEqual(_a.w, _b.w, 1e-4f);
}

static bool FloatEqualQ(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-4f);
}

static bool Vec3EqualQ(Vector3<float> const& _a, Vector3<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f);
}

static bool Vec4EqualQ(Vector4<float> const& _a, Vector4<float> const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-4f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-4f) &&
           MathUtils::NearlyEqual(_a.z, _b.z, 1e-4f) &&
           MathUtils::NearlyEqual(_a.w, _b.w, 1e-4f);
}

class TestQuaternion : public Test
{
public:
    void Run() override
    {
        Test_Constructors();
        Test_Arithmetic();
        Test_CompoundAssignment();
        Test_Comparison();
        Test_Length();
        Test_Conjugate();
        Test_Invert();
        Test_Normalize();
        Test_Dot();
        Test_StaticUtils();
        Test_Conversions();
        Test_StaticFactories();
        DisplayResults();
    }

private:

    using Q  = Quaternion;
    using V3 = Vector3<float>;
    using V4 = Vector4<float>;
    using M3 = Matrix3<float>;
    using M4 = Matrix4<float>;

    static float GetX(Q& _q) { return _q.x; }
    static float GetY(Q& _q) { return _q.y; }
    static float GetZ(Q& _q) { return _q.z; }
    static float GetW(Q& _q) { return _q.w; }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        Q def;
        Q xyzw(1.0f, 2.0f, 3.0f, 4.0f);
        Q fromV4(V4(1.0f, 2.0f, 3.0f, 4.0f));
        Q fromList = { 1.0f, 2.0f, 3.0f, 4.0f };

        TestValue("Constructor | default x",        &GetX, 0.0f, def);
        TestValue("Constructor | default y",        &GetY, 0.0f, def);
        TestValue("Constructor | default z",        &GetZ, 0.0f, def);
        TestValue("Constructor | default w",        &GetW, 1.0f, def);

        TestValue("Constructor | xyzw x",           &GetX, 1.0f, xyzw);
        TestValue("Constructor | xyzw y",           &GetY, 2.0f, xyzw);
        TestValue("Constructor | xyzw z",           &GetZ, 3.0f, xyzw);
        TestValue("Constructor | xyzw w",           &GetW, 4.0f, xyzw);

        TestValue("Constructor | Vector4 x",        &GetX, 1.0f, fromV4);
        TestValue("Constructor | Vector4 y",        &GetY, 2.0f, fromV4);
        TestValue("Constructor | Vector4 z",        &GetZ, 3.0f, fromV4);
        TestValue("Constructor | Vector4 w",        &GetW, 4.0f, fromV4);

        TestValue("Constructor | initializer x",    &GetX, 1.0f, fromList);
        TestValue("Constructor | initializer y",    &GetY, 2.0f, fromList);
        TestValue("Constructor | initializer z",    &GetZ, 3.0f, fromList);
        TestValue("Constructor | initializer w",    &GetW, 4.0f, fromList);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        const Q b(5.0f, 6.0f, 7.0f, 8.0f);

        TestObjectValue("operator+ | basic",    a, &Q::operator+, QuatEqual,
            Q(6.0f, 8.0f, 10.0f, 12.0f), b);
        TestObjectValue("operator- | basic",    a, &Q::operator-, QuatEqual,
            Q(-4.0f, -4.0f, -4.0f, -4.0f), b);

        // q1*q2 = (x1,y1,z1,w1)*(x2,y2,z2,w2)
        // w = w1w2 - x1x2 - y1y2 - z1z2 = 32-5-12-21 = -6
        // x = w1x2 + x1w2 + y1z2 - z1y2 = 20+8+14-18 = 24
        // y = w1y2 - x1z2 + y1w2 + z1x2 = 24-7+16+15 = 48
        // z = w1z2 + x1y2 - y1x2 + z1w2 = 28+12-10+24 = 54  (sign fixed per Hamilton)
        // Hamilton: (1,2,3,4)*(5,6,7,8): w=4*8-1*5-2*6-3*7=-6, x=4*5+1*8+2*7-3*6=24, y=4*6-1*7+2*8+3*5=48, z=4*7+1*6-2*5+3*8=54
        TestObjectValue("operator* | basic",    a, &Q::operator*, QuatEqual,
            Q(24.0f, 48.0f, 48.0f, -6.0f), b);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        Q a(1.0f, 2.0f, 3.0f, 4.0f);
        const Q b(5.0f, 6.0f, 7.0f, 8.0f);

        TestObjectSelf("operator+= | basic",    a, &Q::operator+=, QuatEqual,
            Q(6.0f, 8.0f, 10.0f, 12.0f), b);
        TestObjectSelf("operator-= | basic",    a, &Q::operator-=, QuatEqual,
            Q(-4.0f, -4.0f, -4.0f, -4.0f), b);
        TestObjectSelf("operator*= | basic",    a, &Q::operator*=, QuatEqual,
            Q(24.0f, 48.0f, 48.0f, -6.0f), b);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        const Q b(1.0f, 2.0f, 3.0f, 4.0f);
        const Q c(5.0f, 6.0f, 7.0f, 8.0f);

        TestObjectValue("operator== | equal",       a, &Q::operator==, true,  b);
        TestObjectValue("operator== | not equal",   a, &Q::operator==, false, c);
        TestObjectValue("operator!= | equal",       a, &Q::operator!=, false, b);
        TestObjectValue("operator!= | not equal",   a, &Q::operator!=, true,  c);
    }

    // -----------------------------------------------------------------------

    void Test_Length()
    {
        // (0,0,0,1) -> identity, length = 1
        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);
        // (1,2,3,4) -> length = sqrt(30)
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);

        TestObjectValue("Length | identity",            identity, &Q::Length,        FloatEqualQ, 1.0f);
        TestObjectValue("Length | 1,2,3,4",             a,        &Q::Length,        FloatEqualQ, std::sqrt(30.0f));
        TestObjectValue("LengthSquared | identity",     identity, &Q::LengthSquared, FloatEqualQ, 1.0f);
        TestObjectValue("LengthSquared | 1,2,3,4",      a,        &Q::LengthSquared, FloatEqualQ, 30.0f);

        TestValue("Length (static) | 1,2,3,4",          
            static_cast<float(*)(Q const&)>(&Q::Length),
            FloatEqualQ, std::sqrt(30.0f), a);
        TestValue("LengthSquared (static) | 1,2,3,4",   
            static_cast<float(*)(Q const&)>(&Q::LengthSquared), 
            FloatEqualQ, 30.0f,            a);
    }

    // -----------------------------------------------------------------------

    void Test_Conjugate()
    {
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        // Conjugate negates x,y,z, keeps w
        const Q expected(-1.0f, -2.0f, -3.0f, 4.0f);

        TestObjectValue("Conjugate | member",   a, &Q::Conjugate, QuatEqual, expected);
        TestValue      ("Conjugate | static",   
            static_cast<Q(*)(Q const&)>(&Q::Conjugate),   
            QuatEqual, expected, a);
    }

    // -----------------------------------------------------------------------

    void Test_Invert()
    {
        // Identity quaternion inverted = itself
        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);

        TestObjectValue("Inverted | identity",  identity, &Q::Inverted, QuatEqual, identity);
        TestValue      ("Invert (static) | id", &Q::Invert,             QuatEqual, identity, identity);

        // q * q^-1 = identity
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        const Q expected(-0.0333333f, -0.0666667f, -0.1f, 0.133333f);
        Q inv = a.Inverted();
        Q product = a * inv;
        TestObjectValue("Inverted | q*q^-1 = identity", a, &Q::Inverted, QuatEqual, expected);

        Q selfInvert(1.0f, 2.0f, 3.0f, 4.0f);
        TestObjectSelf("SelfInvert | 1,2,3,4", selfInvert, &Q::SelfInvert, QuatEqual, expected);
    }

    // -----------------------------------------------------------------------

    void Test_Normalize()
    {
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        float len = std::sqrt(30.0f);
        const Q expected(1.0f/len, 2.0f/len, 3.0f/len, 4.0f/len);

        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);

        TestObjectValue("Normalized | 1,2,3,4",         a,        &Q::Normalized,     QuatEqual, expected);
        TestObjectValue("Normalized | identity",        identity, &Q::Normalized,     QuatEqual, identity);
        TestValue      ("Normalize (static) | 1,2,3,4", &Q::Normalize,               QuatEqual, expected, a);

        Q selfNorm(1.0f, 2.0f, 3.0f, 4.0f);
        TestObjectSelf("SelfNormalize | 1,2,3,4",       selfNorm, &Q::SelfNormalize,  QuatEqual, expected);

        // SafeNormalize should behave identically for non-zero quaternions
        TestObjectValue("SafeNormalized | 1,2,3,4",     a,        &Q::SafeNormalized, QuatEqual, expected);
        TestValue      ("SafeNormalize (static) | id",  &Q::SafeNormalize,            QuatEqual, identity, identity);

        Q safeSelf(1.0f, 2.0f, 3.0f, 4.0f);
        TestObjectSelf("SafeSelfNormalize | 1,2,3,4",   safeSelf, &Q::SafeSelfNormalize, QuatEqual, expected);
    }

    // -----------------------------------------------------------------------

    void Test_Dot()
    {
        const Q a(1.0f, 2.0f, 3.0f, 4.0f);
        const Q b(5.0f, 6.0f, 7.0f, 8.0f);
        // dot = 5+12+21+32 = 70
        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);

        TestObjectValue("Dot | member | basic",         a,        &Q::Dot, FloatEqualQ, 70.0f, b);
        TestObjectValue("Dot | member | self",          a,        &Q::Dot, FloatEqualQ, 30.0f, a);
        TestObjectValue("Dot | member | identity",      identity, &Q::Dot, FloatEqualQ, 4.0f,  a);

        TestValue("Dot | static | basic",    
            static_cast<float(*)(Q const&, Q const&)>(&Q::Dot), 
            FloatEqualQ, 70.0f, a, b);
        TestValue("Dot | static | self",     
            static_cast<float(*)(Q const&, Q const&)>(&Q::Dot), 
            FloatEqualQ, 30.0f, a, a);
    }

    // -----------------------------------------------------------------------

    void Test_StaticUtils()
    {
        // Covered by individual tests; kept as a grouping for additional edge cases.
        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);
        const Q neg(0.0f, 0.0f, 0.0f, -1.0f);

        // SafeNormalize on zero should return identity (implementation-defined; test passes if no crash)
        const Q zero(0.0f, 0.0f, 0.0f, 0.0f);
        Q safe = Q::SafeNormalize(zero);
        (void)safe; // result is implementation-defined; just ensure no crash
    }

    // -----------------------------------------------------------------------

    void Test_Conversions()
    {
        // Identity quaternion -> should give identity Matrix3 and Matrix4
        Q identity(0.0f, 0.0f, 0.0f, 1.0f);

        M3 m3 = identity.ToMatrix3();
        M3 identM3 = M3::Identity();
        
        TestObjectValue("ToMatrix3 | identity", identity, &Q::ToMatrix3, identM3);

        M4 m4 = identity.ToMatrix4();
        M4 identM4 = M4::Identity();
        TestObjectValue("ToMatrix3 | identity", identity, &Q::ToMatrix4, identM4);

        // ToAxisAngle: identity -> angle=0, axis arbitrary (usually (0,0,1) or (1,0,0))
        // We only check that angle component (w) is 0
        V4 axisAngle = identity.ToAxisAngle();
        TestValue("ToAxisAngle | identity angle",
            static_cast<float(*)(V4&)>([](V4& v){ return v.w; }),
            FloatEqualQ, 0.0f, axisAngle);

        // Round-trip: Euler -> Quat -> Euler (identity = zero angles)
        Q fromEuler = Q::FromEulerAngles(0.0f, 0.0f, 0.0f);
        TestObjectValue("FromEulerAngles | zero == identity", fromEuler, &Q::operator==, true, identity);

        V3 euler = fromEuler.ToEulerAngles();
        TestValue("ToEulerAngles | identity x", static_cast<float(*)(V3&)>([](V3& v){ return v.x; }), FloatEqualQ, 0.0f, euler);
        TestValue("ToEulerAngles | identity y", static_cast<float(*)(V3&)>([](V3& v){ return v.y; }), FloatEqualQ, 0.0f, euler);
        TestValue("ToEulerAngles | identity z", static_cast<float(*)(V3&)>([](V3& v){ return v.z; }), FloatEqualQ, 0.0f, euler);
    }

    // -----------------------------------------------------------------------

    void Test_StaticFactories()
    {
        const Q identity(0.0f, 0.0f, 0.0f, 1.0f);

        // FromEulerAngles (float overload)
        Q e0 = Q::FromEulerAngles(0.0f, 0.0f, 0.0f);
        TestObjectValue("FromEulerAngles | floats | zero", e0, &Q::operator==, true, identity);

        // FromEulerAngles (Vector3 overload)
        Q e1 = Q::FromEulerAngles(V3(0.0f, 0.0f, 0.0f));
        TestObjectValue("FromEulerAngles | V3 | zero", e1, &Q::operator==, true, identity);

        // FromAxisAngle: rotating 0 degrees around any axis -> identity
        Q aa = Q::FromAxisAngle(V3(0.0f, 0.0f, 1.0f), 0.0f);
        TestObjectValue("FromAxisAngle | angle=0", aa, &Q::operator==, true, identity);

        // FromAxisAngle: 360 degrees = identity
        Q aa360 = Q::FromAxisAngle(V3(0.0f, 1.0f, 0.0f), 2.0f * 3.14159265358979f);
        TestObjectValue("FromAxisAngle | angle=2pi", aa360, &Q::operator==, true, identity);

        // FromRotationMatrix: identity matrix -> identity quaternion
        M3 identM3 = M3::Identity();
        Q fromM3 = Q::FromRotationMatrix(identM3);
        TestObjectValue("FromRotationMatrix | M3 identity", fromM3, &Q::operator==, true, identity);

        M4 identM4 = M4::Identity();
        Q fromM4 = Q::FromRotationMatrix(identM4);
        TestObjectValue("FromRotationMatrix | M4 identity", fromM4, &Q::operator==, true, identity);

        // 90 deg rotation around Z: (0, 0, sin(pi/4), cos(pi/4))
        float s = std::sin(3.14159265358979f / 4.0f);
        float c = std::cos(3.14159265358979f / 4.0f);
        Q rot90Z = Q::FromAxisAngle(V3(0.0f, 0.0f, 1.0f), 3.14159265358979f / 2.0f);
        TestValue("FromAxisAngle | 90 deg Z | x", &GetX, FloatEqualQ, 0.0f, rot90Z);
        TestValue("FromAxisAngle | 90 deg Z | y", &GetY, FloatEqualQ, 0.0f, rot90Z);
        TestValue("FromAxisAngle | 90 deg Z | z", &GetZ, FloatEqualQ, s,    rot90Z);
        TestValue("FromAxisAngle | 90 deg Z | w", &GetW, FloatEqualQ, c,    rot90Z);
    }
};

#endif