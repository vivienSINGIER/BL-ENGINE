#ifndef VECTOR2_TEST_H_DEFINED
#define VECTOR2_TEST_H_DEFINED

#include "Test.hpp"
#include "../Core/Math/Vector/Vector2.h"

using V2 = Vector2<float>;

static bool Vec2Equal(V2 const& _a, V2 const& _b)
{
    return MathUtils::NearlyEqual(_a.x, _b.x, 1e-5f) &&
           MathUtils::NearlyEqual(_a.y, _b.y, 1e-5f);
}

static bool FloatEqualV2(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-5f);
}

class TestVector2 : public Test
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
        Test_Reflect();
        Test_Project();
        Test_Perpendicular();
        Test_DeltaAngle();
        Test_StaticUtils();
        Test_Swizzle();
        Test_Subscript();
        DisplayResults();
    }

private:

    static float GetX(V2& _v) { return _v.x; }
    static float GetY(V2& _v) { return _v.y; }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        V2 def;
        V2 xy(1.0f, 2.0f);
        V2 scalar(3.0f);
        V2 zero = V2::Zero();
        V2 one  = V2::One();
        
        TestValue("Constructor | default x",  &GetX, 0.0f, def);
        TestValue("Constructor | default y",  &GetY, 0.0f, def);
        TestValue("Constructor | xy x",       &GetX, 1.0f, xy);
        TestValue("Constructor | xy y",       &GetY, 2.0f, xy);
        TestValue("Constructor | scalar x",   &GetX, 3.0f, scalar);
        TestValue("Constructor | scalar y",   &GetY, 3.0f, scalar);
        TestValue("Zero | x",                 &GetX, 0.0f, zero);
        TestValue("Zero | y",                 &GetY, 0.0f, zero);
        TestValue("One | x",                  &GetX, 1.0f, one);
        TestValue("One | y",                  &GetY, 1.0f, one);
    }

    // -----------------------------------------------------------------------

    void Test_Arithmetic()
    {
        const V2 a(3.0f, 4.0f);
        const V2 b(1.0f, 2.0f);
        TestObjectValue("operator+ | basic",        a, &V2::operator+,  Vec2Equal, V2(4.0f, 6.0f), b);
        TestObjectValue("operator- | basic",        a, 
            static_cast<V2(V2::*)(V2 const&) const>(&V2::operator-),  
            Vec2Equal, V2(2.0f, 2.0f), b);
        
        TestValue("operator* | scalar reverse",
            static_cast<V2(*)(float, V2 const&)>(&operator*),
            Vec2Equal, V2(6.0f, 8.0f), 2.0f, a);
        TestObjectValue("operator* | basic",        a, 
            static_cast<V2(V2::*)(V2 const&) const>(&V2::operator*),  
            Vec2Equal, V2(3.0f, 8.0f), b);
        TestObjectValue("operator/ | basic",        a, 
            static_cast<V2(V2::*)(V2 const&) const>(&V2::operator/), 
            Vec2Equal, V2(3.0f, 2.0f), b);

        TestObjectValue("operator* | scalar",       a, 
            static_cast<V2(V2::*)(float) const>(&V2::operator*),   
            Vec2Equal, V2(6.0f, 8.0f),  2.0f);
        TestObjectValue("operator/ | scalar",       a, 
            static_cast<V2(V2::*)(float) const>(&V2::operator/),   
            Vec2Equal, V2(1.5f, 2.0f),  2.0f);

        // zero cases
        const V2 zero(0.0f);
        TestObjectValue("operator+ | zero",         a, &V2::operator+,  Vec2Equal, a, zero);
        TestObjectValue("operator- | self",         a, 
            static_cast<V2(V2::*)(V2 const&) const>(&V2::operator-),  
            Vec2Equal, V2(0.0f), a);

        // negative
        const V2 neg(-1.0f, -2.0f);
        TestObjectValue("operator+ | negative",     a, &V2::operator+,  Vec2Equal, V2(2.0f, 2.0f), neg);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        V2 a(3.0f, 4.0f);
        V2 b(1.0f, 2.0f);

        TestObjectSelf("operator+= | basic",        a, &V2::operator+=, Vec2Equal, V2(4.0f, 6.0f), b);
        TestObjectSelf("operator-= | basic",        a, &V2::operator-=, Vec2Equal, V2(2.0f, 2.0f), b);
        TestObjectSelf("operator*= | basic",        a,
            static_cast<V2&(V2::*)(V2 const&)>(&V2::operator*=), 
            Vec2Equal, V2(3.0f, 8.0f), b);
        TestObjectSelf("operator/= | basic",        a, 
            static_cast<V2&(V2::*)(V2 const&)>(&V2::operator/=), 
            Vec2Equal, V2(3.0f, 2.0f), b);
        TestObjectSelf("operator*= | scalar",       a, 
            static_cast<V2&(V2::*)(V2 const&)>(&V2::operator*=), 
            Vec2Equal, V2(6.0f, 8.0f), 2.0f);
        TestObjectSelf("operator/= | scalar",       a, 
            static_cast<V2&(V2::*)(V2 const&)>(&V2::operator/=), 
            Vec2Equal, V2(1.5f, 2.0f), 2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const V2 a(1.0f, 2.0f);
        const V2 b(1.0f, 2.0f);
        const V2 c(3.0f, 4.0f);

        TestObjectValue("operator== | equal",       a, &V2::operator==, true,  b);
        TestObjectValue("operator== | not equal",   a, &V2::operator==, false, c);
        TestObjectValue("operator!= | equal",       a, &V2::operator!=, false, b);
        TestObjectValue("operator!= | not equal",   a, &V2::operator!=, true,  c);

        V2 zero(0.0f);
        TestObjectValue("IsNull | zero",            zero, &V2::IsNull, true);
        TestObjectValue("IsNull | non-zero",        a,    &V2::IsNull, false);
    }

    // -----------------------------------------------------------------------

    void Test_Length()
    {
        V2 a(3.0f, 4.0f);      // length = 5
        V2 unit(1.0f, 0.0f);   // length = 1
        V2 zero(0.0f);

        TestObjectValue("Length | 3,4",             a,    &V2::Length,        FloatEqualV2, 5.0f);
        TestObjectValue("Length | unit",            unit, &V2::Length,        FloatEqualV2, 1.0f);
        TestObjectValue("Length | zero",            zero, &V2::Length,        FloatEqualV2, 0.0f);
        TestObjectValue("LengthSquared | 3,4",      a,    &V2::LengthSquared, FloatEqualV2, 25.0f);
        TestObjectValue("LengthSquared | unit",     unit, &V2::LengthSquared, FloatEqualV2, 1.0f);
        TestObjectValue("LengthSquared | zero",     zero, &V2::LengthSquared, FloatEqualV2, 0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Normalize()
    {
        V2 a(3.0f, 4.0f);
        V2 unit(1.0f, 0.0f);

        TestObjectValue("Normalized | 3,4",         a,    &V2::Normalized,    Vec2Equal, V2(0.6f, 0.8f));
        TestObjectValue("Normalized | unit",        unit, &V2::Normalized,    Vec2Equal, V2(1.0f, 0.0f));
        TestObjectSelf ("SelfNormalize | 3,4",      a,    &V2::SelfNormalize, Vec2Equal, V2(0.6f, 0.8f));
        TestObjectSelf ("SelfNormalize | unit",     unit, &V2::SelfNormalize, Vec2Equal, V2(1.0f, 0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DotProduct()
    {
        const V2 a(3.0f, 4.0f);
        const V2 b(1.0f, 2.0f);
        const V2 perp(-4.0f, 3.0f);  // perpendicular to a
        const V2 zero(0.0f);

        TestValue("Dot | static | basic",         static_cast<float(*)(V2 const&, V2 const&)>(&V2::Dot), FloatEqualV2, 11.0f,  a, b);
        TestValue("Dot | static | perpendicular", static_cast<float(*)(V2 const&, V2 const&)>(&V2::Dot), FloatEqualV2, 0.0f,   a, perp);
        TestValue("Dot | static | zero",          static_cast<float(*)(V2 const&, V2 const&)>(&V2::Dot), FloatEqualV2, 0.0f,   a, zero);
        TestValue("Dot | static | self",          static_cast<float(*)(V2 const&, V2 const&)>(&V2::Dot), FloatEqualV2, 25.0f,  a, a);
        
        TestObjectValue("Dot | member | basic",         a, static_cast<float(V2::*)(V2 const&) const>(&V2::Dot), FloatEqualV2, 11.0f, b);
        TestObjectValue("Dot | member | perpendicular", a, static_cast<float(V2::*)(V2 const&) const>(&V2::Dot), FloatEqualV2, 0.0f,  perp);
        TestObjectValue("Dot | member | zero",          a, static_cast<float(V2::*)(V2 const&) const>(&V2::Dot), FloatEqualV2, 0.0f,  zero);
        TestObjectValue("Dot | member | self",          a, static_cast<float(V2::*)(V2 const&) const>(&V2::Dot), FloatEqualV2, 25.0f, a);
    }

    // -----------------------------------------------------------------------

    void Test_Reflect()
    {
        const V2 v(1.0f, -1.0f);
        const V2 normalUp(0.0f, 1.0f);
        const V2 normalRight(1.0f, 0.0f);

        TestObjectValue("Reflect | floor normal",   v, &V2::Reflect, Vec2Equal, V2(1.0f,  1.0f), normalUp);
        TestObjectValue("Reflect | wall normal",    v, &V2::Reflect, Vec2Equal, V2(-1.0f,-1.0f), normalRight);
    }

    // -----------------------------------------------------------------------

    void Test_Project()
    {
        const V2 a(3.0f, 4.0f);
        const V2 xAxis(1.0f, 0.0f);
        const V2 yAxis(0.0f, 1.0f);

        TestObjectValue("Project | onto x axis",    a, &V2::Project, Vec2Equal, V2(3.0f, 0.0f), xAxis);
        TestObjectValue("Project | onto y axis",    a, &V2::Project, Vec2Equal, V2(0.0f, 4.0f), yAxis);
        TestObjectValue("Project | onto self",      a, &V2::Project, Vec2Equal, a,                          a);
    }

    // -----------------------------------------------------------------------

    void Test_Perpendicular()
    {
        V2 a(1.0f, 0.0f);
        V2 b(3.0f, 4.0f);

        TestObjectValue("Perpendicular | x axis",   a, &V2::Perpendicular, Vec2Equal, V2(0.0f, 1.0f));
        // perp of (3,4) should be (-4,3) or (4,-3) depending on convention — adjust if needed
        TestObjectValue("Perpendicular | 3,4",      b, &V2::Perpendicular, Vec2Equal, V2(-4.0f, 3.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DeltaAngle()
    {
        const V2 right(1.0f, 0.0f);
        const V2 up   (0.0f, 1.0f);
        const V2 left (-1.0f, 0.0f);

        TestObjectValue("DeltaAngle | 90 deg",      right, &V2::DeltaAngle, FloatEqualV2,  MathUtils::HALF_PI, up);
        TestObjectValue("DeltaAngle | -90 deg",     up,    &V2::DeltaAngle, FloatEqualV2, -MathUtils::HALF_PI, right);
        TestObjectValue("DeltaAngle | 180 deg",     right, &V2::DeltaAngle, FloatEqualV2,  MathUtils::PI,      left);
        TestObjectValue("DeltaAngle | 0 deg",       right, &V2::DeltaAngle, FloatEqualV2,  0.0f,               right);
    }

    // -----------------------------------------------------------------------

    void Test_StaticUtils()
    {
        const V2 a( 2.0f, -3.0f);
        const V2 b(-1.0f,  4.0f);

        TestValue("Min | basic",    &V2::Min,   Vec2Equal, V2(-1.0f, -3.0f), a, b);
        TestValue("Max | basic",    &V2::Max,   Vec2Equal, V2( 2.0f,  4.0f), a, b);
        TestValue("Abs | mixed",    static_cast<V2(*)(V2 const&)>(&V2::Abs),   Vec2Equal, V2( 2.0f,  3.0f), a);

        const V2 v(3.0f, -1.0f);
        const V2 lo(0.0f,  0.0f);
        const V2 hi(2.0f,  2.0f);
        TestValue("Clamp | basic",  static_cast<V2(*)(V2 const&, V2 const&, V2 const&)>(&V2::Clamp), Vec2Equal, V2(2.0f, 0.0f), v, lo, hi);

        TestValue("Normalize | 3,4",&V2::Normalize, Vec2Equal, V2(0.6f, 0.8f), V2(3.0f, 4.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle()
    {
        V2 v(1.0f, 2.0f);
        TestObjectValue("yx | basic", v, &V2::yx, Vec2Equal, V2(2.0f, 1.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        V2 v(3.0f, 7.0f);
        TestValue("operator[] | x", &GetX, 3.0f, v);
        TestValue("operator[] | y", &GetY, 7.0f, v);
    }
};

#endif