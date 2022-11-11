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

#pragma once
#include <GL/glew.h>

#include <cmath>

// PI Values
float constexpr PI = 3.14159265359f;
float constexpr TWO_PI = 2.0f * PI;
float constexpr HALF_PI = 0.5f * PI;
float constexpr QUARTER_PI = 0.25f * PI;
float constexpr EIGHTH_PI = 0.125f * PI;
float constexpr SIXTEENTH_PI = 0.0625f * PI;

// Angle Conversions
float constexpr DEG_TO_RAD = PI / 180.0f;
float constexpr RAD_TO_DEG = 180.0f / PI;

// Epsilon Value
float constexpr EPSILON = 0.00001f;

namespace Math
{
    template <typename T>
    T Lerp(T const& src, T const& dst, float interpolant)
    {
        //// This might cause jittering
        // T tmp = src;
        //
        // if (interpolant < EPSILON)
        //  tmp = dst;
        // else
        //  tmp += (dst - src) * interpolant;
        // return tmp;
        //

        // Replaced jittering
        return (interpolant >= 1.0f) ? dst : src + (dst - src) * interpolant;
    }

    template <typename T>
    T RandomRange(T min, T max)
    {
        return min + static_cast<T>(((float)rand() / (float)RAND_MAX) * static_cast<float>(max - min));
    }

    template <typename T>
    T ToRadians(T degrees)
    {
        return degrees * DEG_TO_RAD;
    }

    template <typename T>
    T ToDegrees(T degrees)
    {
        return degrees * RAD_TO_DEG;
    }

}  // namespace Math

template <typename T>
GLfloat const* ValuePtrFloat(T const& obj)
{
    return reinterpret_cast<GLfloat const*>(&obj);
}

template <typename T>
GLint const* ValuePtrInt(T const& obj)
{
    return reinterpret_cast<GLint const*>(&obj);
}
