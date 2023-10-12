/******************************************************************************/
/*!
  All content © 2017 DigiPen Institute of Technology Singapore.
  All Rights Reserved

  File Name: Vector.h
  Author(s): Wang Yuxuan Jason
  Contribution for the file: 100%
  Brief Description:

    This file contains the interface for the templated Vector type.
*/
/******************************************************************************/

module;

#include <cmath>
#include <concepts>
#include <iostream>

export module Vector;

import Math;

export template <ArithmeticType BaseType, unsigned Components>
    requires(Components > 1)
class Vector
{
    static constexpr bool IsBaseTypeFloat = std::is_floating_point_v<BaseType>;

public:

    /// <summary>
    /// Construct from parameter pack of arbitrary arithmetic types
    /// </summary>
    template <ArithmeticType... ArgTypes>
    explicit constexpr Vector(const ArgTypes... args)
        requires(Components >= sizeof...(ArgTypes))
        : values { static_cast<BaseType>(args)... }
    {
    }

    /// <summary>
    ///  Construct from another vector of different type and dimension
    /// </summary>
    template <ArithmeticType OtherBaseType, unsigned OtherComponents>
    explicit constexpr Vector(const Vector<OtherBaseType, OtherComponents>& other)
        : values {}
    {
        constexpr unsigned MinComponents = std::min(Components, OtherComponents);

        for (unsigned i = 0; i < MinComponents; ++i)
        {
            values[i] = static_cast<BaseType>(other[i]);
        }
    }

    /// <summary>
    /// Construct from another vector of different type and smaller dimension with literal fillers
    /// </summary>
    template <ArithmeticType OtherBaseType, unsigned OtherComponents, ArithmeticType... ArgTypes>
    explicit constexpr Vector(const Vector<OtherBaseType, OtherComponents>& other, const ArgTypes... args)
        requires(Components >= sizeof...(args) + OtherComponents && sizeof...(args) != 0)
        : values {}
    {
        for (unsigned i = 0; i < OtherComponents; ++i)
        {
            values[i] = static_cast<BaseType>(other[i]);
        }

        const BaseType argsArray[sizeof...(args)] { static_cast<BaseType>(args)... };

        for (unsigned i = 0; i < sizeof...(args); ++i)
        {
            values[i + OtherComponents] = argsArray[i];
        }
    }

    static consteval Vector Base(const unsigned index)
    {
        Vector result {};
        result.values[index] = static_cast<BaseType>(1);
        return result;
    }

#pragma region BASE

    // Sets all values to be the same
    void Fill(const BaseType fillValue)
    {
        for (BaseType& value : values)
        {
            value = value;
        }
    }

    BaseType& operator[](const unsigned index) { return values[index]; }
    constexpr BaseType operator[](const unsigned index) const { return values[index]; }

    BaseType* GetData() { return values; }
    constexpr BaseType* GetData() const { return values; }

    constexpr bool operator==(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (values[i] != other.values[i])
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator!=(const Vector& other) const { return !((*this) == other); }
    constexpr bool operator<(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] < other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator<=(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] <= other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator>(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] > other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator>=(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] >= other.values[i]))
            {
                return false;
            }
        }
        return true;
    }

#pragma endregion BASE

#pragma region MATH

    Vector& operator+=(const Vector& other)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] += other.values[i];
        }
        return *this;
    }
    Vector& operator-=(const Vector& other)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] -= other.values[i];
        }
        return *this;
    }
    Vector& operator*=(const Vector& other)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] *= other.values[i];
        }
        return *this;
    }
    Vector& operator/=(const Vector& other)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] /= other.values[i];
        }
        return *this;
    }
    Vector& operator*=(const BaseType value)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] *= value;
        }
        return *this;
    }
    Vector& operator/=(const BaseType value)
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            values[i] /= value;
        }
        return *this;
    }

    constexpr Vector operator-() const { return Vector { *this } *= static_cast<BaseType>(-1); }
    constexpr Vector operator+(const Vector& other) const { return Vector { *this } += other; }
    constexpr Vector operator-(const Vector& other) const { return Vector { *this } -= other; }
    constexpr Vector operator*(const Vector& other) const { return Vector { *this } *= other; }
    constexpr Vector operator/(const Vector& other) const { return Vector { *this } /= other; }
    constexpr Vector operator*(const BaseType value) const { return Vector { *this } *= value; }
    constexpr Vector operator/(const BaseType value) const { return Vector { *this } /= value; }

    // Utilities
    constexpr BaseType Dot(const Vector& other) const
    {
        BaseType result = static_cast<BaseType>(0);
        for (unsigned i = 0; i < Components; ++i)
        {
            result += values[i] * other.values[i];
        }
        return result;
    }
    constexpr BaseType SquareLength() const { return Dot(*this); }
    constexpr BaseType Length() const
        requires(IsBaseTypeFloat)
    {
        return static_cast<BaseType>(std::sqrtf(SquareLength()));
    }
    constexpr BaseType SquareDistance(const Vector& other) { return (other - *this).SquareLength(); }
    constexpr BaseType Distance(const Vector& other)
        requires(IsBaseTypeFloat)
    {
        return (other - *this).Length();
    }

    void Normalize() { (*this) /= this->Length(); }
    constexpr Vector Normalized() const { return Vector { *this } /= this->Length(); }

    constexpr Vector Projection(const Vector& other) const
        requires(IsBaseTypeFloat)
    {
        const Vector otherNormalized = other.Normalized();
        const BaseType length = Dot(otherNormalized);
        return otherNormalized * length;
    }

    constexpr Vector Min(const Vector& other) const
    {
        Vector result {};
        for (unsigned i = 0; i < Components; ++i)
        {
            result.values[i] = std::min(values[i], other.values[i]);
        }
        return result;
    }
    constexpr Vector Max(const Vector& other) const
    {
        Vector result {};
        for (unsigned i = 0; i < Components; ++i)
        {
            result.values[i] = std::max(values[i], other.values[i]);
        }
        return result;
    }

#pragma endregion MATH

#pragma region VECTOR2

    constexpr BaseType Angle() const
        requires(Components == 2 && std ::is_floating_point_v<BaseType>)
    {
        return std::atan2f(-values[1], -values[0]) * RAD_TO_DEG + 180;
    }

    constexpr Vector Rotate(const BaseType angle) const
        requires(Components == 2 && std ::is_floating_point_v<BaseType>)
    {
        return Vector { values[0] * std::cos(angle) - values[1] * std::sin(angle), values[0] * std::sin(angle) + values[1] * std::cos(angle) };
    }

#pragma endregion VECTOR2

#pragma region VECTOR3

    constexpr Vector Cross(const Vector& other) const
        requires(Components == 3)
    {
        return Vector {
            values[1] * other.values[2] - values[2] * other.values[1],
            values[2] * other.values[0] - values[0] * other.values[2],
            values[0] * other.values[1] - values[1] * other.values[0],
        };
    }

    constexpr Vector<BaseType, 2> PolarAngles() const
        requires(Components >= 3 && IsBaseTypeFloat)
    {
        Vector tmp = Normalized();
        Vector<BaseType, 2> result;

        result[0] = std::acosf(tmp[1]);
        result[1] = std::atan2f(-tmp[2], tmp[0]);

        return result;
    }

    constexpr Vector RotateX(const BaseType angle) const
        requires(Components >= 3 && std ::is_floating_point_v<BaseType>)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { values[0], values[1] * c + -s * values[2], s * values[1] + c * values[2] };
    }

    constexpr Vector RotateY(const BaseType angle) const
        requires(Components >= 3 && std ::is_floating_point_v<BaseType>)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { c * values[0] + s * values[2], values[1], -s * values[0] + c * values[2] };
    }

    constexpr Vector RotateZ(const BaseType angle) const
        requires(Components >= 3 && IsBaseTypeFloat)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { c * values[0] + -s * values[1], s * values[0] + c * values[1], values[2] };
    }

#pragma region VECTOR3

protected:

    BaseType values[Components];
};

export template <ArithmeticType BaseType, unsigned Components>
constexpr Vector<BaseType, Components> operator*(const BaseType value, const Vector<BaseType, Components>& other)
{
    return other * value;
}

export template <ArithmeticType BaseType, unsigned Components>
std::ostream& operator<<(std::ostream& os, const Vector<BaseType, Components>& vector)
{
    os << "[ ";

    for (unsigned i = 0; i < Components - 1; ++i)
    {
        os << vector[i] << ", ";
    }

    os << vector[Components - 1] << " ]" << std::endl;

    return os;
}
