#ifndef VECTOR2_TEST_H_DEFINED
#define VECTOR2_TEST_H_DEFINED

#include "Test.hpp"
#include "../Render/Common/Math/Vector2.h"

static bool Vec2Equal(Vector2<float> const& _a, Vector2<float> const& _b)
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

    static float GetX(Vector2<float>& _v) { return _v.x; }
    static float GetY(Vector2<float>& _v) { return _v.y; }

    // -----------------------------------------------------------------------

    void Test_Constructors()
    {
        Vector2<float> def;
        Vector2<float> xy(1.0f, 2.0f);
        Vector2<float> scalar(3.0f);
        Vector2<float> zero = Vector2<float>::Zero();
        Vector2<float> one  = Vector2<float>::One();
        
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
        const Vector2<float> a(3.0f, 4.0f);
        const Vector2<float> b(1.0f, 2.0f);
        TestObjectValue("operator+ | basic",        a, &Vector2<float>::operator+,  Vec2Equal, Vector2<float>(4.0f, 6.0f), b);
        TestObjectValue("operator- | basic",        a, &Vector2<float>::operator-,  Vec2Equal, Vector2<float>(2.0f, 2.0f), b);
        
        TestValue("operator* | scalar reverse",
            static_cast<Vector2<float>(*)(float, Vector2<float> const&)>(&operator*),
            Vec2Equal, Vector2<float>(6.0f, 8.0f), 2.0f, a);
        TestObjectValue("operator* | basic",        a, 
            static_cast<Vector2<float>(Vector2<float>::*)(Vector2<float> const&) const>(&Vector2<float>::operator*),  
            Vec2Equal, Vector2<float>(3.0f, 8.0f), b);
        TestObjectValue("operator/ | basic",        a, 
            static_cast<Vector2<float>(Vector2<float>::*)(Vector2<float> const&) const>(&Vector2<float>::operator/), 
            Vec2Equal, Vector2<float>(3.0f, 2.0f), b);

        TestObjectValue("operator* | scalar",       a, 
            static_cast<Vector2<float>(Vector2<float>::*)(float) const>(&Vector2<float>::operator*),   
            Vec2Equal, Vector2<float>(6.0f, 8.0f),  2.0f);
        TestObjectValue("operator/ | scalar",       a, 
            static_cast<Vector2<float>(Vector2<float>::*)(float) const>(&Vector2<float>::operator/),   
            Vec2Equal, Vector2<float>(1.5f, 2.0f),  2.0f);

        // zero cases
        const Vector2<float> zero(0.0f);
        TestObjectValue("operator+ | zero",         a, &Vector2<float>::operator+,  Vec2Equal, a, zero);
        TestObjectValue("operator- | self",         a, &Vector2<float>::operator-,  Vec2Equal, Vector2<float>(0.0f), a);

        // negative
        const Vector2<float> neg(-1.0f, -2.0f);
        TestObjectValue("operator+ | negative",     a, &Vector2<float>::operator+,  Vec2Equal, Vector2<float>(2.0f, 2.0f), neg);
    }

    // -----------------------------------------------------------------------

    void Test_CompoundAssignment()
    {
        Vector2<float> a(3.0f, 4.0f);
        Vector2<float> b(1.0f, 2.0f);

        TestObjectSelf("operator+= | basic",        a, &Vector2<float>::operator+=, Vec2Equal, Vector2<float>(4.0f, 6.0f), b);
        TestObjectSelf("operator-= | basic",        a, &Vector2<float>::operator-=, Vec2Equal, Vector2<float>(2.0f, 2.0f), b);
        TestObjectSelf("operator*= | basic",        a,
            static_cast<Vector2<float>&(Vector2<float>::*)(Vector2<float> const&)>(&Vector2<float>::operator*=), 
            Vec2Equal, Vector2<float>(3.0f, 8.0f), b);
        TestObjectSelf("operator/= | basic",        a, 
            static_cast<Vector2<float>&(Vector2<float>::*)(Vector2<float> const&)>(&Vector2<float>::operator/=), 
            Vec2Equal, Vector2<float>(3.0f, 2.0f), b);
        TestObjectSelf("operator*= | scalar",       a, 
            static_cast<Vector2<float>&(Vector2<float>::*)(Vector2<float> const&)>(&Vector2<float>::operator*=), 
            Vec2Equal, Vector2<float>(6.0f, 8.0f), 2.0f);
        TestObjectSelf("operator/= | scalar",       a, 
            static_cast<Vector2<float>&(Vector2<float>::*)(Vector2<float> const&)>(&Vector2<float>::operator/=), 
            Vec2Equal, Vector2<float>(1.5f, 2.0f), 2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Comparison()
    {
        const Vector2<float> a(1.0f, 2.0f);
        const Vector2<float> b(1.0f, 2.0f);
        const Vector2<float> c(3.0f, 4.0f);

        TestObjectValue("operator== | equal",       a, &Vector2<float>::operator==, true,  b);
        TestObjectValue("operator== | not equal",   a, &Vector2<float>::operator==, false, c);
        TestObjectValue("operator!= | equal",       a, &Vector2<float>::operator!=, false, b);
        TestObjectValue("operator!= | not equal",   a, &Vector2<float>::operator!=, true,  c);

        Vector2<float> zero(0.0f);
        TestObjectValue("IsNull | zero",            zero, &Vector2<float>::IsNull, true);
        TestObjectValue("IsNull | non-zero",        a,    &Vector2<float>::IsNull, false);
    }

    // -----------------------------------------------------------------------

    void Test_Length()
    {
        Vector2<float> a(3.0f, 4.0f);      // length = 5
        Vector2<float> unit(1.0f, 0.0f);   // length = 1
        Vector2<float> zero(0.0f);

        TestObjectValue("Length | 3,4",             a,    &Vector2<float>::Length,        FloatEqualV2, 5.0f);
        TestObjectValue("Length | unit",            unit, &Vector2<float>::Length,        FloatEqualV2, 1.0f);
        TestObjectValue("Length | zero",            zero, &Vector2<float>::Length,        FloatEqualV2, 0.0f);
        TestObjectValue("LengthSquared | 3,4",      a,    &Vector2<float>::LengthSquared, FloatEqualV2, 25.0f);
        TestObjectValue("LengthSquared | unit",     unit, &Vector2<float>::LengthSquared, FloatEqualV2, 1.0f);
        TestObjectValue("LengthSquared | zero",     zero, &Vector2<float>::LengthSquared, FloatEqualV2, 0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Normalize()
    {
        Vector2<float> a(3.0f, 4.0f);
        Vector2<float> unit(1.0f, 0.0f);

        TestObjectValue("Normalized | 3,4",         a,    &Vector2<float>::Normalized,    Vec2Equal, Vector2<float>(0.6f, 0.8f));
        TestObjectValue("Normalized | unit",        unit, &Vector2<float>::Normalized,    Vec2Equal, Vector2<float>(1.0f, 0.0f));
        TestObjectSelf ("SelfNormalize | 3,4",      a,    &Vector2<float>::SelfNormalize, Vec2Equal, Vector2<float>(0.6f, 0.8f));
        TestObjectSelf ("SelfNormalize | unit",     unit, &Vector2<float>::SelfNormalize, Vec2Equal, Vector2<float>(1.0f, 0.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DotProduct()
    {
        const Vector2<float> a(3.0f, 4.0f);
        const Vector2<float> b(1.0f, 2.0f);
        const Vector2<float> perp(-4.0f, 3.0f);  // perpendicular to a
        const Vector2<float> zero(0.0f);

        TestValue("Dot | basic",                    &Vector2<float>::Dot, FloatEqualV2, 11.0f,  a, b);
        TestValue("Dot | perpendicular",            &Vector2<float>::Dot, FloatEqualV2, 0.0f,   a, perp);
        TestValue("Dot | zero",                     &Vector2<float>::Dot, FloatEqualV2, 0.0f,   a, zero);
        TestValue("Dot | self",                     &Vector2<float>::Dot, FloatEqualV2, 25.0f,  a, a);
    }

    // -----------------------------------------------------------------------

    void Test_Reflect()
    {
        const Vector2<float> v(1.0f, -1.0f);
        const Vector2<float> normalUp(0.0f, 1.0f);
        const Vector2<float> normalRight(1.0f, 0.0f);

        TestObjectValue("Reflect | floor normal",   v, &Vector2<float>::Reflect, Vec2Equal, Vector2<float>(1.0f,  1.0f), normalUp);
        TestObjectValue("Reflect | wall normal",    v, &Vector2<float>::Reflect, Vec2Equal, Vector2<float>(-1.0f,-1.0f), normalRight);
    }

    // -----------------------------------------------------------------------

    void Test_Project()
    {
        const Vector2<float> a(3.0f, 4.0f);
        const Vector2<float> xAxis(1.0f, 0.0f);
        const Vector2<float> yAxis(0.0f, 1.0f);

        TestObjectValue("Project | onto x axis",    a, &Vector2<float>::Project, Vec2Equal, Vector2<float>(3.0f, 0.0f), xAxis);
        TestObjectValue("Project | onto y axis",    a, &Vector2<float>::Project, Vec2Equal, Vector2<float>(0.0f, 4.0f), yAxis);
        TestObjectValue("Project | onto self",      a, &Vector2<float>::Project, Vec2Equal, a,                          a);
    }

    // -----------------------------------------------------------------------

    void Test_Perpendicular()
    {
        Vector2<float> a(1.0f, 0.0f);
        Vector2<float> b(3.0f, 4.0f);

        TestObjectValue("Perpendicular | x axis",   a, &Vector2<float>::Perpendicular, Vec2Equal, Vector2<float>(0.0f, 1.0f));
        // perp of (3,4) should be (-4,3) or (4,-3) depending on convention — adjust if needed
        TestObjectValue("Perpendicular | 3,4",      b, &Vector2<float>::Perpendicular, Vec2Equal, Vector2<float>(-4.0f, 3.0f));
    }

    // -----------------------------------------------------------------------

    void Test_DeltaAngle()
    {
        const Vector2<float> right(1.0f, 0.0f);
        const Vector2<float> up   (0.0f, 1.0f);
        const Vector2<float> left (-1.0f, 0.0f);

        TestObjectValue("DeltaAngle | 90 deg",      right, &Vector2<float>::DeltaAngle, FloatEqualV2,  MathUtils::HALF_PI, up);
        TestObjectValue("DeltaAngle | -90 deg",     up,    &Vector2<float>::DeltaAngle, FloatEqualV2, -MathUtils::HALF_PI, right);
        TestObjectValue("DeltaAngle | 180 deg",     right, &Vector2<float>::DeltaAngle, FloatEqualV2,  MathUtils::PI,      left);
        TestObjectValue("DeltaAngle | 0 deg",       right, &Vector2<float>::DeltaAngle, FloatEqualV2,  0.0f,               right);
    }

    // -----------------------------------------------------------------------

    void Test_StaticUtils()
    {
        const Vector2<float> a( 2.0f, -3.0f);
        const Vector2<float> b(-1.0f,  4.0f);

        TestValue("Min | basic",    &Vector2<float>::Min,   Vec2Equal, Vector2<float>(-1.0f, -3.0f), a, b);
        TestValue("Max | basic",    &Vector2<float>::Max,   Vec2Equal, Vector2<float>( 2.0f,  4.0f), a, b);
        TestValue("Abs | mixed",    &Vector2<float>::Abs,   Vec2Equal, Vector2<float>( 2.0f,  3.0f), a);

        const Vector2<float> v(3.0f, -1.0f);
        const Vector2<float> lo(0.0f,  0.0f);
        const Vector2<float> hi(2.0f,  2.0f);
        TestValue("Clamp | basic",  &Vector2<float>::Clamp, Vec2Equal, Vector2<float>(2.0f, 0.0f), v, lo, hi);

        TestValue("Normalize | 3,4",&Vector2<float>::Normalize, Vec2Equal, Vector2<float>(0.6f, 0.8f), Vector2<float>(3.0f, 4.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Swizzle()
    {
        Vector2<float> v(1.0f, 2.0f);
        TestObjectValue("yx | basic", v, &Vector2<float>::yx, Vec2Equal, Vector2<float>(2.0f, 1.0f));
    }

    // -----------------------------------------------------------------------

    void Test_Subscript()
    {
        Vector2<float> v(3.0f, 7.0f);
        TestValue("operator[] | x", &GetX, 3.0f, v);
        TestValue("operator[] | y", &GetY, 7.0f, v);
    }
};

#endif