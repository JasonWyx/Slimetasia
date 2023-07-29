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
public:

    template <ArithmeticType... ArgTypes>
    explicit constexpr Vector(const ArgTypes... args)
        requires(Components >= sizeof...(ArgTypes))
        : values { static_cast<BaseType>(args)... }
    {
    }

    // Initialize from another vector of smaller or equal components with different base types
    template <ArithmeticType OtherBaseType, unsigned OtherComponents, ArithmeticType... ArgTypes>
    explicit constexpr Vector(const Vector<OtherBaseType, OtherComponents>& other, const ArgTypes... args)
        requires(Components >= sizeof...(args) + OtherComponents)
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

    // Sets all values to be the same
    void Fill(const BaseType fillValue)
    {
        for (BaseType& value : values)
        {
            value = value;
        }
    }

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

    Vector operator-() const { return Vector {} -= *this; }
    Vector operator+(const Vector& other) const { return Vector { *this } += other; }
    Vector operator-(const Vector& other) const { return Vector { *this } -= other; }
    Vector operator*(const Vector& other) const { return Vector { *this } *= other; }
    Vector operator/(const Vector& other) const { return Vector { *this } /= other; }
    Vector operator*(const BaseType value) const { return Vector { *this } *= value; }
    Vector operator/(const BaseType value) const { return Vector { *this } /= value; }

    bool operator==(const Vector& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (values[i] != other[i])
            {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Vector& other) const { return !((*this) == other); }
    bool operator<(Vector const& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] < other[i]))
            {
                return false;
            }
        }
        return true;
    }
    bool operator<=(Vector const& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] <= other[i]))
            {
                return false;
            }
        }
        return true;
    }
    bool operator>(Vector const& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] > other[i]))
            {
                return false;
            }
        }
        return true;
    }
    bool operator>=(Vector const& other) const
    {
        for (unsigned i = 0; i < Components; ++i)
        {
            if (!(values[i] >= other[i]))
            {
                return false;
            }
        }
        return true;
    }

    BaseType& operator[](const unsigned index) { return values[index]; }
    constexpr BaseType operator[](const unsigned index) const { return values[index]; }

    BaseType* GetData() { return values; }
    const BaseType* GetData() const { return values; }

    // Conversion to other base types
    template <typename OtherBaseType, unsigned OtherComponents = Components>
    explicit constexpr operator Vector<OtherBaseType, OtherComponents>() const
        requires(Components >= OtherComponents)
    {
        Vector<OtherBaseType, OtherComponents> converted;

        for (unsigned i = 0; i < OtherComponents; ++i)
        {
            converted[i] = static_cast<OtherBaseType>(values[i]);
        }

        return converted;
    }

    // Utilities
    void Zero() { *this = Vector {}; }
    BaseType Dot(const Vector& other) const
    {
        BaseType result = static_cast<BaseType>(0);
        for (unsigned i = 0; i < Components; ++i)
        {
            result += values[i] * other.values[i];
        }
        return result;
    }

    BaseType SquareLength() const { return Dot(*this); }
    BaseType Length() const
        requires(std::is_floating_point_v<BaseType>)
    {
        return static_cast<BaseType>(std::sqrtf(SquareLength()));
    }

    BaseType SquareDistance(const Vector& other) { return (other - *this).SquareLength(); }
    BaseType Distance(const Vector& other) { return (other - *this).Length(); }

    void Normalize() { (*this) /= this->Length(); }
    Vector Normalized() const { return Vector { *this } /= this->Length(); }

    Vector Projection(const Vector& other) const
        requires(std::is_floating_point_v<BaseType>)
    {
        const Vector otherNormalized = other.Normalized();
        const BaseType length = Dot(otherNormalized);
        return otherNormalized * length;
    }

    // Components[2] Specifics
    BaseType Angle() const
        requires(std::is_floating_point_v<BaseType> && Components == 2)
    {
        return std::atan2f(-values[1], -values[0]) * RAD_TO_DEG + 180;
    }

    Vector Rotate(const BaseType angle) const
        requires(std::is_floating_point_v<BaseType> && Components == 2)
    {
        return Vector { values[0] * std::cos(angle) - values[1] * std::sin(angle), values[0] * std::sin(angle) + values[1] * std::cos(angle) };
    }

    // Components[3] Specifics

    Vector Cross(const Vector& other) const
        requires(Components == 3)
    {
        return Vector { values[1] * other.values[2] - values[2] * other.values[1], values[2] * other.values[0] - values[0] * other.values[2],
            values[0] * other.values[1] - values[1] * other.values[0] };
    }

    Vector<BaseType, 2> PolarAngles() const
        requires(std::is_floating_point_v<BaseType> && Components == 3)
    {
        Vector tmp = Normalized();
        Vector<BaseType, 2> result;

        result[0] = std::acosf(tmp[1]);
        result[1] = std::atan2f(-tmp[2], tmp[0]);

        return result;
    }

    Vector RotateX(const BaseType angle) const
        requires(std::is_floating_point_v<BaseType> && Components == 3)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { values[0], values[1] * c + -s * values[2], s * values[1] + c * values[2] };
    }

    Vector RotateY(const BaseType angle) const
        requires(std::is_floating_point_v<BaseType> && Components == 3)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { c * values[0] + s * values[2], values[1], -s * values[0] + c * values[2] };
    }

    Vector RotateZ(const BaseType angle) const
        requires(std::is_floating_point_v<BaseType> && Components == 3)
    {
        BaseType c = std::cosf(angle);
        BaseType s = std::sinf(angle);

        return Vector { c * values[0] + -s * values[1], s * values[0] + c * values[1], values[2] };
    }

    // Static Utilities

    template <unsigned Index>
        requires(Index < Components)
    static constexpr Vector Base = MakeBase<Index>();

private:

    template <unsigned Index>
    static constexpr Vector MakeBase()
        requires(Index < Components)
    {
        return MakeBase(MakeNumberPack<Index, 0> {});
    }

    template <int... PackNumbers>
    static constexpr Vector MakeBase(const NumberPack<PackNumbers...>&)
    {
        return Vector { PackNumbers..., static_cast<BaseType>(1) };
    }

    BaseType values[Components];
};

export template <ArithmeticType BaseType, unsigned Components>
Vector<BaseType, Components> operator*(BaseType value, const Vector<BaseType, Components>& other)
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
