#ifndef TESTS_MATH_MATHMETHODS_HPP
#define TESTS_MATH_MATHMETHODS_HPP

#include "Test.hpp"
#include "../Render/Common/Math/MathUtils.hpp"

static bool FloatEqual(float const& _a, float const& _b)
{
    return MathUtils::NearlyEqual(_a, _b, 1e-5f);
}

class TestMathMethods : public Test
{
public:
    void Run() override
    {
        Test_Min();
        Test_Max();
        Test_Clamp();
        Test_Abs();
        Test_Sign();
        Test_Floor();
        Test_Ceil();
        Test_Round();
        Test_Sqrt();
        Test_ISqrt();
        Test_Factorial();
        Test_Pow();
        Test_Exp();
        Test_Log();
        Test_Angles();
        Test_Trig();
        Test_InverseTrig();
        Test_Utilities();
        DisplayResults();
    }

private:

    // -----------------------------------------------------------------------

    void Test_Min()
    {
        printf("\n");
        TestValue("Min | smaller first",        &MathUtils::Min<int>,   1,      1, 2);
        TestValue("Min | larger first",         &MathUtils::Min<int>,   1,      2, 1);
        TestValue("Min | equal",                &MathUtils::Min<int>,   5,      5, 5);
        TestValue("Min | negatives",            &MathUtils::Min<int>,   -5,     -5, -2);
        TestValue("Min | float",                &MathUtils::Min<float>, FloatEqual, 1.5f, 1.5f, 2.5f);
    }

    // -----------------------------------------------------------------------

    void Test_Max()
    {
        printf("\n");
        TestValue("Max | smaller first",        &MathUtils::Max<int>,   2,      1, 2);
        TestValue("Max | larger first",         &MathUtils::Max<int>,   2,      2, 1);
        TestValue("Max | equal",                &MathUtils::Max<int>,   5,      5, 5);
        TestValue("Max | negatives",            &MathUtils::Max<int>,   -2,     -5, -2);
        TestValue("Max | float",                &MathUtils::Max<float>, FloatEqual, 2.5f, 1.5f, 2.5f);
    }

    // -----------------------------------------------------------------------

    void Test_Clamp()
    {
        printf("\n");
        TestValue("Clamp | within range",       &MathUtils::Clamp<int>, 5,      5, 0, 10);
        TestValue("Clamp | below min",          &MathUtils::Clamp<int>, 0,      -5, 0, 10);
        TestValue("Clamp | above max",          &MathUtils::Clamp<int>, 10,     15, 0, 10);
        TestValue("Clamp | at min",             &MathUtils::Clamp<int>, 0,      0, 0, 10);
        TestValue("Clamp | at max",             &MathUtils::Clamp<int>, 10,     10, 0, 10);
        TestValue("Clamp | negative range",     &MathUtils::Clamp<int>, -3,     -3, -5, -1);
    }

    // -----------------------------------------------------------------------

    void Test_Abs()
    {
        printf("\n");
        TestValue("Abs | positive",             &MathUtils::Abs<int>,   5,      5);
        TestValue("Abs | negative",             &MathUtils::Abs<int>,   5,      -5);
        TestValue("Abs | zero",                 &MathUtils::Abs<int>,   0,      0);
        TestValue("Abs | float negative",       &MathUtils::Abs<float>, FloatEqual, 3.14f, -3.14f);
    }

    // -----------------------------------------------------------------------

    void Test_Sign()
    {
        printf("\n");
        TestValue("Sign | positive",            &MathUtils::Sign<int>,  1,      5);
        TestValue("Sign | negative",            &MathUtils::Sign<int>,  -1,     -5);
        TestValue("Sign | zero",                &MathUtils::Sign<int>,  0,      0);
        TestValue("Sign | float positive",      &MathUtils::Sign<float>,1,      3.14f);
        TestValue("Sign | float negative",      &MathUtils::Sign<float>,-1,     -3.14f);
    }

    // -----------------------------------------------------------------------

    void Test_Floor()
    {
        printf("\n");
        TestValue("Floor | positive",           &MathUtils::Floor,      2,      2.9f);
        TestValue("Floor | negative",           &MathUtils::Floor,      -3,     -2.1f);
        TestValue("Floor | exact",              &MathUtils::Floor,      2,      2.0f);
        TestValue("Floor | zero",               &MathUtils::Floor,      0,      0.0f);
        TestValue("Floor | negative exact",     &MathUtils::Floor,      -2,     -2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Ceil()
    {
        printf("\n");
        TestValue("Ceil | positive",            &MathUtils::Ceil,       3,      2.1f);
        TestValue("Ceil | negative",            &MathUtils::Ceil,       -2,     -2.9f);
        TestValue("Ceil | exact",               &MathUtils::Ceil,       2,      2.0f);
        TestValue("Ceil | zero",                &MathUtils::Ceil,       0,      0.0f);
        TestValue("Ceil | negative exact",      &MathUtils::Ceil,       -2,     -2.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Round()
    {
        printf("\n");
        TestValue("Round | round up",           &MathUtils::Round,      3,      2.5f);
        TestValue("Round | round down",         &MathUtils::Round,      2,      2.4f);
        TestValue("Round | negative round up",  &MathUtils::Round,      -3,     -2.5f);
        TestValue("Round | negative round down",&MathUtils::Round,      -2,     -2.4f);
        TestValue("Round | exact",              &MathUtils::Round,      2,      2.0f);
        TestValue("Round | zero",               &MathUtils::Round,      0,      0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Sqrt()
    {
        printf("\n");
        TestValue("Sqrt | 4",                   &MathUtils::Sqrt, FloatEqual, 2.0f,       4.0f);
        TestValue("Sqrt | 0",                   &MathUtils::Sqrt, FloatEqual, 0.0f,       0.0f);
        TestValue("Sqrt | 1",                   &MathUtils::Sqrt, FloatEqual, 1.0f,       1.0f);
        TestValue("Sqrt | 2",                   &MathUtils::Sqrt, FloatEqual, 1.41421f,   2.0f);
        TestValue("Sqrt | 9",                   &MathUtils::Sqrt, FloatEqual, 3.0f,       9.0f);
    }

    // -----------------------------------------------------------------------

    void Test_ISqrt()
    {
        printf("\n");
        TestValue("ISqrt | 4",                  &MathUtils::ISqrt, FloatEqual, 0.5f,      4.0f);
        TestValue("ISqrt | 1",                  &MathUtils::ISqrt, FloatEqual, 1.0f,      1.0f);
        TestValue("ISqrt | 16",                 &MathUtils::ISqrt, FloatEqual, 0.25f,     16.0f);
        TestValue("ISqrt | 100",                &MathUtils::ISqrt, FloatEqual, 0.1f,      100.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Factorial()
    {
        printf("\n");
        TestValue("Factorial | 0",              &MathUtils::Factorial,  1,  (uint8)0);
        TestValue("Factorial | 1",              &MathUtils::Factorial,  1,  (uint8)1);
        TestValue("Factorial | 5",              &MathUtils::Factorial,  120,(uint8)5);
        TestValue("Factorial | 7",              &MathUtils::Factorial,  5040,(uint8)7);
    }

    // -----------------------------------------------------------------------

    void Test_Pow()
    {
        printf("\n");
        TestValue("Pow | 2^0",                  &MathUtils::Pow, FloatEqual, 1.0f,   2.0f, (uint8)0);
        TestValue("Pow | 2^1",                  &MathUtils::Pow, FloatEqual, 2.0f,   2.0f, (uint8)1);
        TestValue("Pow | 2^8",                  &MathUtils::Pow, FloatEqual, 256.0f, 2.0f, (uint8)8);
        TestValue("Pow | 3^3",                  &MathUtils::Pow, FloatEqual, 27.0f,  3.0f, (uint8)3);
        TestValue("Pow | 0^5",                  &MathUtils::Pow, FloatEqual, 0.0f,   0.0f, (uint8)5);
    }

    // -----------------------------------------------------------------------

    void Test_Exp()
    {
        printf("\n");
        TestValue("Exp | 0",                    &MathUtils::Exp, FloatEqual, 1.0f,       0.0f);
        TestValue("Exp | 1",                    &MathUtils::Exp, FloatEqual, 2.71828f,   1.0f);
        TestValue("Exp | 2",                    &MathUtils::Exp, FloatEqual, 7.38906f,   2.0f);
        TestValue("Exp | -1",                   &MathUtils::Exp, FloatEqual, 0.36788f,   -1.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Log()
    {
        printf("\n");
        TestValue("Log2 | 1",                   &MathUtils::Log2,  FloatEqual, 0.0f,    1.0f);
        TestValue("Log2 | 2",                   &MathUtils::Log2,  FloatEqual, 1.0f,    2.0f);
        TestValue("Log2 | 8",                   &MathUtils::Log2,  FloatEqual, 3.0f,    8.0f);

        TestValue("Log10 | 1",                  &MathUtils::Log10, FloatEqual, 0.0f,    1.0f);
        TestValue("Log10 | 10",                 &MathUtils::Log10, FloatEqual, 1.0f,    10.0f);
        TestValue("Log10 | 100",                &MathUtils::Log10, FloatEqual, 2.0f,    100.0f);

        TestValue("Ln | 1",                     &MathUtils::Ln,    FloatEqual, 0.0f,    1.0f);
        TestValue("Ln | E",                     &MathUtils::Ln,    FloatEqual, 1.0f,    MathUtils::E);

        TestValue("LogN | base 2",              &MathUtils::LogN,  FloatEqual, 3.0f,    8.0f,  2.0f);
        TestValue("LogN | base 10",             &MathUtils::LogN,  FloatEqual, 2.0f,    100.0f,10.0f);
        TestValue("LogN | base 3",              &MathUtils::LogN,  FloatEqual, 2.0f,    9.0f,  3.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Angles()
    {
        printf("\n");
        TestValue("ToRadians | 0",              &MathUtils::ToRadians, FloatEqual, 0.0f,           0.0f);
        TestValue("ToRadians | 180",            &MathUtils::ToRadians, FloatEqual, MathUtils::PI,  180.0f);
        TestValue("ToRadians | 90",             &MathUtils::ToRadians, FloatEqual, MathUtils::HALF_PI, 90.0f);
        TestValue("ToRadians | 360",            &MathUtils::ToRadians, FloatEqual, MathUtils::TWO_PI,  360.0f);

        TestValue("ToDegrees | 0",              &MathUtils::ToDegrees, FloatEqual, 0.0f,   0.0f);
        TestValue("ToDegrees | PI",             &MathUtils::ToDegrees, FloatEqual, 180.0f, MathUtils::PI);
        TestValue("ToDegrees | PI/2",           &MathUtils::ToDegrees, FloatEqual, 90.0f,  MathUtils::HALF_PI);
        TestValue("ToDegrees | TWO_PI",         &MathUtils::ToDegrees, FloatEqual, 360.0f, MathUtils::TWO_PI);

        TestValue("WrapAngle | no wrap",        &MathUtils::WrapAngle, FloatEqual, 1.0f,           1.0f);
        TestValue("WrapAngle | over TWO_PI",    &MathUtils::WrapAngle, FloatEqual, 0.0f,           MathUtils::TWO_PI);
        TestValue("WrapAngle | negative",       &MathUtils::WrapAngle, FloatEqual, MathUtils::PI,  -MathUtils::PI);
    }

    // -----------------------------------------------------------------------

    void Test_Trig()
    {
        printf("\n");
        TestValue("Sin | 0",                    &MathUtils::Sin, FloatEqual, 0.0f,   0.0f);
        TestValue("Sin | PI/2",                 &MathUtils::Sin, FloatEqual, 1.0f,   MathUtils::HALF_PI);
        TestValue("Sin | PI",                   &MathUtils::Sin, FloatEqual, 0.0f,   MathUtils::PI);
        TestValue("Sin | -PI/2",                &MathUtils::Sin, FloatEqual, -1.0f,  -MathUtils::HALF_PI);

        TestValue("Cos | 0",                    &MathUtils::Cos, FloatEqual, 1.0f,   0.0f);
        TestValue("Cos | PI/2",                 &MathUtils::Cos, FloatEqual, 0.0f,   MathUtils::HALF_PI);
        TestValue("Cos | PI",                   &MathUtils::Cos, FloatEqual, -1.0f,  MathUtils::PI);

        TestValue("Tan | 0",                    &MathUtils::Tan, FloatEqual, 0.0f,   0.0f);
        TestValue("Tan | PI/4",                 &MathUtils::Tan, FloatEqual, 1.0f,   MathUtils::PI / 4.0f);
        TestValue("Tan | -PI/4",                &MathUtils::Tan, FloatEqual, -1.0f,  -MathUtils::PI / 4.0f);
    }

    // -----------------------------------------------------------------------

    void Test_InverseTrig()
    {
        printf("\n");
        TestValue("Asin | 0",                   &MathUtils::Asin, FloatEqual, 0.0f,           0.0f);
        TestValue("Asin | 1",                   &MathUtils::Asin, FloatEqual, MathUtils::HALF_PI, 1.0f);
        TestValue("Asin | -1",                  &MathUtils::Asin, FloatEqual, -MathUtils::HALF_PI, -1.0f);

        TestValue("Acos | 1",                   &MathUtils::Acos, FloatEqual, 0.0f,           1.0f);
        TestValue("Acos | 0",                   &MathUtils::Acos, FloatEqual, MathUtils::HALF_PI, 0.0f);
        TestValue("Acos | -1",                  &MathUtils::Acos, FloatEqual, MathUtils::PI,  -1.0f);

        TestValue("Atan | 0",                   &MathUtils::Atan, FloatEqual, 0.0f,           0.0f);
        TestValue("Atan | 1",                   &MathUtils::Atan, FloatEqual, MathUtils::PI / 4.0f, 1.0f);
        TestValue("Atan | -1",                  &MathUtils::Atan, FloatEqual, -MathUtils::PI / 4.0f, -1.0f);

        TestValue("Atan2 | (0, 1)",             &MathUtils::Atan2, FloatEqual, 0.0f,           0.0f, 1.0f);
        TestValue("Atan2 | (1, 1)",             &MathUtils::Atan2, FloatEqual, MathUtils::PI / 4.0f, 1.0f, 1.0f);
        TestValue("Atan2 | (1, 0)",             &MathUtils::Atan2, FloatEqual, MathUtils::HALF_PI, 1.0f, 0.0f);
        TestValue("Atan2 | (-1, 0)",            &MathUtils::Atan2, FloatEqual, -MathUtils::HALF_PI, -1.0f, 0.0f);
    }

    // -----------------------------------------------------------------------

    void Test_Utilities()
    {
        printf("\n");
        TestValue("IsPowerOfTwo | 1",           &MathUtils::IsPowerOfTwo,       true,   1);
        TestValue("IsPowerOfTwo | 2",           &MathUtils::IsPowerOfTwo,       true,   2);
        TestValue("IsPowerOfTwo | 4",           &MathUtils::IsPowerOfTwo,       true,   4);
        TestValue("IsPowerOfTwo | 3",           &MathUtils::IsPowerOfTwo,       false,  3);
        TestValue("IsPowerOfTwo | 0",           &MathUtils::IsPowerOfTwo,       false,  0);

        TestValue("NextPowerOfTwo | 1",         &MathUtils::NextPowerOfTwo,     2,      1);
        TestValue("NextPowerOfTwo | 3",         &MathUtils::NextPowerOfTwo,     4,      3);
        TestValue("NextPowerOfTwo | 4",         &MathUtils::NextPowerOfTwo,     8,      4);
        TestValue("NextPowerOfTwo | 7",         &MathUtils::NextPowerOfTwo,     8,      7);

        TestValue("PreviousPowerOfTwo | 2",     &MathUtils::PreviousPowerOfTwo, 1,      2);
        TestValue("PreviousPowerOfTwo | 5",     &MathUtils::PreviousPowerOfTwo, 4,      5);
        TestValue("PreviousPowerOfTwo | 8",     &MathUtils::PreviousPowerOfTwo, 4,      8);
        TestValue("PreviousPowerOfTwo | 9",     &MathUtils::PreviousPowerOfTwo, 8,      9);
    }
};

#endif