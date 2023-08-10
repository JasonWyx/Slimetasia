module;

#include <cmath>
#include <concepts>

export module Quaternion;

import Math;
import Vector;
import Matrix;

export template <ArithmeticType BaseType>
class Quaternion
{
    using VectorBase = Vector<float, 4>;

public:

    /// <summary>
    /// Construct from parameter pack of arbitrary arithmetic types
    /// </summary>
    template <ArithmeticType... ArgTypes>
    explicit constexpr Quaternion(const ArgTypes... args)
        requires(sizeof...(ArgTypes) <= 4)
        : values { static_cast<BaseType>(args)... }
    {
    }

    /// <summary>
    ///  Construct from another vector of different type and dimension
    /// </summary>
    template <ArithmeticType OtherBaseType, unsigned OtherComponents>
    explicit constexpr Quaternion(const Vector<OtherBaseType, OtherComponents>& other)
        requires(OtherComponents <= 4)
        : values {}
    {
        constexpr unsigned MinComponents = std::min(4, OtherComponents);

        for (unsigned i = 0; i < MinComponents; ++i)
        {
            values[i] = static_cast<BaseType>(other[i]);
        }
    }

    /// <summary>
    /// Construct from angle axis values
    /// </summary>
    /// <param name="vector">About axis</param>
    /// <param name="angle">Rotation angle</param>
    explicit constexpr Quaternion(const Vector<BaseType, 3>& vector, const BaseType angle)
        : Quaternion { vector }
    {
        const BaseType alpha = angle * 0.5f;
        const BaseType sinAlpha = std::sinf(alpha);
        values[0] *= sinAlpha;
        values[1] *= sinAlpha;
        values[2] *= sinAlpha;
        values[3] *= std::cosf(alpha);
    }

    static constexpr Quaternion Identity() { return Quaternion { VectorBase::Base<3> }; }

#pragma region BASE

    template<unsigned OtherComponents>
    explicit constexpr operator Vector<BaseType, OtherComponents>() const
    {
        Vector<BaseType, OtherComponents> tmp {};

        constexpr unsigned MinComponents = std::min(4, OtherComponents);

        for (unsigned i = 0; i < MinComponents; ++i)
        {
            tmp[i] = values[i];
        }

        return tmp;
    }

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

    constexpr bool operator==(const Quaternion& other) const
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (values[i] != other.values[i])
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator!=(const Quaternion& other) const { return !((*this) == other); }
    constexpr bool operator<(const Quaternion& other) const
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (!(values[i] < other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator<=(const Quaternion& other) const
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (!(values[i] <= other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator>(const Quaternion& other) const
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (!(values[i] > other.values[i]))
            {
                return false;
            }
        }
        return true;
    }
    constexpr bool operator>=(const Quaternion& other) const
    {
        for (unsigned i = 0; i < 4; ++i)
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

    Quaternion& operator+=(const Quaternion& other)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            values[i] += other.values[i];
        }
        return *this;
    }
    Quaternion& operator-=(const Quaternion& other)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            values[i] -= other.values[i];
        }
        return *this;
    }
    Quaternion& operator/=(const Quaternion& other)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            values[i] /= other.values[i];
        }
        return *this;
    }
    Quaternion& operator*=(const BaseType value)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            values[i] *= value;
        }
        return *this;
    }
    Quaternion& operator/=(const BaseType value)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            values[i] /= value;
        }
        return *this;
    }

    constexpr Quaternion operator-() const { return Quaternion { *this } *= static_cast<BaseType>(-1); }
    constexpr Quaternion operator+(const Quaternion& other) const { return Quaternion { *this } += other; }
    constexpr Quaternion operator-(const Quaternion& other) const { return Quaternion { *this } -= other; }
    constexpr Quaternion operator/(const Quaternion& other) const { return Quaternion { *this } /= other; }
    constexpr Quaternion operator*(const BaseType value) const { return Quaternion { *this } *= value; }
    constexpr Quaternion operator/(const BaseType value) const { return Quaternion { *this } /= value; }

    /// <summary>
    /// Rotates another quaternion
    /// </summary>
    /// <param name="other"></param>
    /// <returns></returns>
    Quaternion& operator*=(const Quaternion& other)
    {
        Quaternion tmp { *this };
        values[0] = tmp[3] * other[0] + tmp[0] * other[3] + tmp[1] * other[2] - tmp[2] * other[1];
        values[1] = tmp[3] * other[1] + tmp[1] * other[3] + tmp[2] * other[0] - tmp[0] * other[2];
        values[2] = tmp[3] * other[2] + tmp[2] * other[3] + tmp[0] * other[1] - tmp[1] * other[0];
        values[3] = tmp[3] * other[3] - tmp[0] * other[0] - tmp[1] * other[1] - tmp[2] * other[2];
        return *this;
    }

    constexpr Quaternion operator*(const Quaternion& other) const { return Quaternion(*this) *= other; }

    // Utilities
    constexpr BaseType Dot(const Quaternion& other) const
    {
        BaseType result = static_cast<BaseType>(0);
        for (unsigned i = 0; i < 4; ++i)
        {
            result += values[i] * other.values[i];
        }
        return result;
    }
    constexpr BaseType SquareLength() const { return Dot(*this); }
    constexpr BaseType Length() const
        requires(std::is_floating_point_v<BaseType>)
    {
        return static_cast<BaseType>(std::sqrtf(SquareLength()));
    }

    void Normalize() { (*this) /= this->Length(); }
    constexpr Quaternion Normalized() const { return Quaternion { *this } /= this->Length(); }


    // Rotate vector
    template <unsigned OtherComponents>
    constexpr Vector<BaseType, OtherComponents> operator*(const Vector<BaseType, OtherComponents>& other) const
        requires(OtherComponents >= 3)
    {
        const BaseType p0 = values[3] * other[0] + values[1] * other[2] - values[2] * other[1];
        const BaseType p1 = values[3] * other[1] + values[2] * other[0] - values[0] * other[2];
        const BaseType p2 = values[3] * other[2] + values[0] * other[1] - values[1] * other[0];
        const BaseType p3 = -values[0] * other[0] - values[1] * other[1] - values[2] * other[2];

        return Vector<BaseType, 4> {
            values[3] * p0 - p1 * values[2] + p2 * values[1] - p3 * values[0],
            values[3] * p1 - p2 * values[0] + p0 * values[2] - p3 * values[1],
            values[3] * p2 - p0 * values[1] + p1 * values[0] - p3 * values[2],
        };
    }

#pragma endregion MATH

    constexpr Quaternion Conjugate() const
    {
        return Quaternion {
            values[0] * static_cast<BaseType>(-1),
            values[1] * static_cast<BaseType>(-1),
            values[2] * static_cast<BaseType>(-1),
            values[3],
        };
    }

    constexpr Quaternion Integrate(const Vector<BaseType, 3>& vector, BaseType scalar)
    {
        Quaternion q { vector * scalar };

        q *= *this;

        values[0] += 0.5f * q[0];
        values[1] += 0.5f * q[1];
        values[2] += 0.5f * q[2];
        values[3] += 0.5f * q[3];
    }
    constexpr Quaternion Invert() const { return Quaternion(*this).Conjugate() /= SquareLength(); }
    constexpr Quaternion Exponent() const
    {
        Quaternion tmp { *this };
        tmp[3] = 0;

        const BaseType a = tmp.Length();
        const BaseType c = std::cosf(a);
        const BaseType s = std::sinf(a) / a;
        
        tmp *= s;
        tmp[3] = c;

        return tmp;
    }
    constexpr Quaternion Logarithm() const
    {
        BaseType theta = std::acosf(values[3]);
        BaseType sinTheta = std::sinf(theta);
        theta /= sinTheta;
        return Quaternion(values[0] * theta, values[1] * theta, values[2] * theta, 0);
    }
    constexpr Vector<BaseType, 3> EulerAngles() const
    {
        return Vector<BaseType, 3> {
            std::atan2f(2 * (values[3] * values[0] + values[1] * values[2]), 1 - 2 * (values[0] * values[0] + values[1] * values[1])),
            std::asinf(2 * (values[3] * values[1] - values[2] * values[0])),
            std::atan2f(2 * (values[3] * values[2] + values[0] * values[1]), 1 - 2 * (values[1] * values[1] + values[2] * values[2])),
        };
    }
    constexpr Matrix<BaseType, 4, 4> EulerTransform() const
    {
        return Matrix<BaseType, 4, 4> {
            1 - 2 * (values[1] * values[1] + values[2] * values[2]),
            2 * (values[0] * values[1] + values[3] * values[2]),
            2 * (values[0] * values[2] - values[3] * values[1]),
            0,
            2 * (values[0] * values[1] - values[3] * values[2]),
            1 - 2 * (values[0] * values[0] + values[2] * values[2]),
            2 * (values[1] * values[2] + values[3] * values[0]),
            0,
            2 * (values[0] * values[2] + values[3] * values[1]),
            2 * (values[1] * values[2] - values[3] * values[0]),
            1 - 2 * (values[0] * values[0] + values[1] * values[1]),
            0,
            0,
            0,
            0,
            1,
        };
    }
    constexpr Quaternion GetConjugate() const { return Quaternion(-values[0], -values[1], -values[2], values[3]); }
    constexpr Quaternion GetInverse() const { return Quaternion(-values[0], -values[1], -values[2], values[3]); }
    constexpr Matrix<BaseType, 3, 3> GetMatrix() const
    {
        BaseType nQ = values[0] * values[0] + values[1] * values[1] + values[2] * values[2] + values[3] * values[3];
        BaseType s = nQ > static_cast<BaseType>(0) ? (static_cast<BaseType>(2) / nQ) : static_cast<BaseType>(0);

        BaseType xs = values[0] * s;
        BaseType ys = values[1] * s;
        BaseType zs = values[2] * s;
        BaseType wxs = values[3] * xs;
        BaseType wys = values[3] * ys;
        BaseType wzs = values[3] * zs;
        BaseType xxs = values[0] * xs;
        BaseType xys = values[0] * ys;
        BaseType xzs = values[0] * zs;
        BaseType yys = values[1] * ys;
        BaseType yzs = values[1] * zs;
        BaseType zys = values[2] * ys;
        BaseType zzs = values[2] * zs;

        return Matrix<BaseType, 3, 3> {
            static_cast<BaseType>(1) - yys - zzs,
            xys - wzs,
            xzs + wys,
            xys + wzs,
            static_cast<BaseType>(1) - xxs - zzs,
            yzs - wxs,
            xzs - wys,
            yzs + wxs,
            static_cast<BaseType>(1) + xxs - yys,
        };
    }

    static Quaternion Slerp(const Quaternion& src, const Quaternion& dst, BaseType t)
    {
        Quaternion v0 { src.Normalized() };
        Quaternion v1 { dst.Normalized() };

        BaseType dot = v0.Dot(v1);

        if (dot < 0)
        {
            v1 = -v1;
            dot = -dot;
        }

        // Calculate coefficients
        BaseType sclp, sclq;
        if ((1.0f - dot) > 0.0001f)  // 0.0001 -> some epsillon
        {
            // Standard case (slerp)
            BaseType omega, sinom;
            omega = std::acosf(dot);  // extract theta from dot product's cos theta
            sinom = std::sinf(omega);
            sclp = std::sinf((1.0f - t) * omega) / sinom;
            sclq = std::sinf(t * omega) / sinom;
        }
        else
        {
            // Very close, do linear interp (because it's faster)
            sclp = 1.0f - t;
            sclq = t;
        }

        return v0 * sclp + v1 * sclq;
    }

private:

    BaseType values[4];
};
