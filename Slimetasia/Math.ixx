/******************************************************************************/
/*!
  All content © 2017 DigiPen Institute of Technology Singapore.
  All Rights Reserved

  File Name: Math.h
  Author(s): Wang Yuxuan Jason
  Contribution for the file: 100%
  Brief Description:

    This file contains the defines for math constants, typedefs as well as
    some utility math functions.
*/
/******************************************************************************/
module;

#ifdef USE_VULKAN
#else
#include <gl/glew.h>
#endif

#include <cmath>
#include <concepts>
#include <numbers>
#include <random>

export module Math;
// import std.compat;

// PI Values
export constexpr float PI = std::numbers::pi_v<float>;
export constexpr float RAD360 = PI * 2;
export constexpr float RAD180 = PI;
export constexpr float RAD90 = PI / 2;
export constexpr float RAD45 = PI / 4;
export constexpr float RAD22_5 = PI / 8;
export constexpr float RAD11_25 = PI / 16;

// Angle Conversions
export constexpr float DEG_TO_RAD = RAD180 / 180.0f;
export constexpr float RAD_TO_DEG = 180.0f / RAD180;

// Epsilon Value
export constexpr float EPSILON = 0.00001f;

// Convenience
export constexpr char MAX_CHAR = (std::numeric_limits<char>::max)();
export constexpr char MIN_CHAR = (std::numeric_limits<char>::min)();
export constexpr unsigned char MAX_UCHAR = (std::numeric_limits<unsigned char>::max)();
export constexpr unsigned char MIN_UCHAR = (std::numeric_limits<unsigned char>::min)();
export constexpr short MAX_SHORT = (std::numeric_limits<short>::max)();
export constexpr short MIN_SHORT = (std::numeric_limits<short>::min)();
export constexpr unsigned short MAX_USHORT = (std::numeric_limits<unsigned short>::max)();
export constexpr unsigned short MIN_USHORT = (std::numeric_limits<unsigned short>::min)();
export constexpr long MAX_LONG = (std::numeric_limits<long>::max)();
export constexpr long MIN_LONG = (std::numeric_limits<long>::min)();
export constexpr unsigned long MAX_ULONG = (std::numeric_limits<unsigned long>::max)();
export constexpr unsigned long MIN_ULONG = (std::numeric_limits<unsigned long>::min)();
export constexpr long long MAX_LONGLONG = (std::numeric_limits<long long>::max)();
export constexpr long long MIN_LONGLONG = (std::numeric_limits<long long>::max)();
export constexpr unsigned long long MAX_ULONGLONG = (std::numeric_limits<unsigned long long>::max)();
export constexpr unsigned long long MIN_ULONGLONG = (std::numeric_limits<unsigned long long>::min)();
export constexpr int MAX_INT = (std::numeric_limits<int>::max)();
export constexpr int MIN_INT = (std::numeric_limits<int>::min)();
export constexpr unsigned int MAX_UINT = (std::numeric_limits<unsigned int>::max)();
export constexpr unsigned int MIN_UINT = (std::numeric_limits<unsigned int>::min)();
export constexpr float MAX_FLOAT = (std::numeric_limits<float>::max)();
export constexpr float MIN_FLOAT = (std::numeric_limits<float>::min)();
export constexpr double MAX_DOUBLE = (std::numeric_limits<double>::max)();
export constexpr double MIN_DOUBLE = (std::numeric_limits<double>::min)();
export constexpr long double MAX_LONGDOUBLE = (std::numeric_limits<long double>::max)();
export constexpr long double MIN_LONGDOUBLE = (std::numeric_limits<long double>::min)();

// Utility concepts
export template <typename T>
concept ArithmeticType = std::is_arithmetic_v<T>;

export template <int... PackNumbers>
struct NumberPack
{
};

template <unsigned Count, int Number, int... PackNumbers>
struct NumberPackHelper : NumberPackHelper<Count - 1, Number, Number, PackNumbers...>
{
};

template <int Number, int... PackNumbers>
struct NumberPackHelper<0, Number, PackNumbers...>
{
    using type = NumberPack<PackNumbers...>;
};

export template<unsigned Count, int Number>
using MakeNumberPack = typename NumberPackHelper<Count, Number>::type;

export namespace Math
{
    export template <typename T>
    T Lerp(T const& src, T const& dst, float interpolant)
    {
        return (interpolant >= 1.0f) ? dst : src + (dst - src) * interpolant;
    }

    export template <typename T>
        requires std::is_arithmetic_v<T>
    T RandomRange(T min, T max)

    {
        std::mt19937_64 generator {};

        return min + static_cast<T>(static_cast<double>(generator()) / generator.max() * (max - min));
    }

    export template <std::floating_point T>
    T ToRadians(T degrees)
    {
        return degrees * DEG_TO_RAD;
    }

    export template <std::floating_point T>
    T ToDegrees(T radians)
    {
        return radians * RAD_TO_DEG;
    }

}  // namespace Math

#ifndef USE_VULKAN

template <typename T>
export GLfloat const* ValuePtrFloat(T const& obj)
{
    return reinterpret_cast<GLfloat const*>(&obj);
}

template <typename T>
export GLint const* ValuePtrInt(T const& obj)
{
    return reinterpret_cast<GLint const*>(&obj);
}

#endif  // !USE_VULKAN
