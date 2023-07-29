module;

#include <cmath>

export module Quaternion;

import Math;
import Vector;
import Matrix;

export template <typename BaseType>
class Quaternion
{
public:

    template <ArithmeticType... ArgTypes>
    explicit Quaternion(const ArgTypes... args)
        requires(sizeof...(ArgTypes) <= 4)
        : values { args... }
    {
    }

    // Construct from vector
    explicit Quaternion(const Vector<BaseType, 3>& vector)
        : values { vector[0], vector[1], vector[2], 0 }
    {
    }
    // Construct from axis angle rotation
    explicit Quaternion(const Vector<BaseType, 3>& vector, const BaseType angle)
    {
        BaseType alpha = angle * 0.5f;
        BaseType sinAlpha = std::sinf(alpha);
        values[0] *= sinAlpha;
        values[1] *= sinAlpha;
        values[2] *= sinAlpha;
        values[3] *= std::cosf(alpha);
    }

    // Conversion to vectors
    explicit operator Vector<BaseType, 3>() const { return Vector<BaseType, 3>(values[0], values[1], values[2]); }
    explicit operator Vector<BaseType, 4>() const { return Vector<BaseType, 4>(values[0], values[1], values[2], values[3]); }

    Quaternion& operator+=(const Quaternion& other)
    {
        values[0] += other[0];
        values[1] += other[1];
        values[2] += other[2];
        values[3] += other[3];
        return *this;
    }
    Quaternion& operator-=(const Quaternion& other)
    {
        values[0] -= other[0];
        values[1] -= other[1];
        values[2] -= other[2];
        values[3] -= other[3];
        return *this;
    }
    Quaternion& operator*=(const Quaternion& other)
    {
        Quaternion<BaseType> tmp(*this);
        values[0] = tmp[3] * other[0] + tmp[0] * other[3] + tmp[1] * other[2] - tmp[2] * other[1];
        values[1] = tmp[3] * other[1] + tmp[1] * other[3] + tmp[2] * other[0] - tmp[0] * other[2];
        values[2] = tmp[3] * other[2] + tmp[2] * other[3] + tmp[0] * other[1] - tmp[1] * other[0];
        values[3] = tmp[3] * other[3] - tmp[0] * other[0] - tmp[1] * other[1] - tmp[2] * other[2];
        return *this;
    }
    Quaternion& operator*=(BaseType other)
    {
        values[0] *= other;
        values[1] *= other;
        values[2] *= other;
        values[3] *= other;
        return *this;
    }
    Quaternion& operator/=(BaseType other)
    {
        values[0] /= other;
        values[1] /= other;
        values[2] /= other;
        values[3] /= other;
        return *this;
    }
    Quaternion operator-() const { return Quaternion(-values[0], -values[1], -values[2], -values[3]); }
    Quaternion operator*(const Quaternion& other) const { return Quaternion<BaseType>(*this) *= other; }
    Quaternion operator+(const Quaternion& other) const { return Quaternion<BaseType>(*this) += other; }
    Quaternion operator-(const Quaternion& other) const { return Quaternion<BaseType>(*this) -= other; }
    Quaternion operator*(BaseType const& other) const { return Quaternion<BaseType>(*this) *= other; }
    Quaternion operator/(BaseType const& other) const { return Quaternion<BaseType>(*this) /= other; }
    bool operator==(const Quaternion& other) const { return values[0] == other[0] && values[1] == other[1] && values[2] == other[2] && values[3] == other[3]; }
    bool operator!=(const Quaternion& other) const { return !(*this == other); }

    BaseType& operator[](const unsigned index) { return values[index]; }

    constexpr BaseType operator[](const unsigned index) const { return values[index]; }

    // Rotate vector
    Vector<BaseType, 3> operator*(const Vector<BaseType, 3>& other) const
    {
        const BaseType p0 = values[3] * other[0] + values[1] * other[2] - values[2] * other[1];
        const BaseType p1 = values[3] * other[1] + values[2] * other[0] - values[0] * other[2];
        const BaseType p2 = values[3] * other[2] + values[0] * other[1] - values[1] * other[0];
        const BaseType p3 = -values[0] * other[0] - values[1] * other[1] - values[2] * other[2];
        return Vector<BaseType, 3> { values[3] * p0 - p1 * values[2] + p2 * values[1] - p3 * values[0], values[3] * p1 - p2 * values[0] + p0 * values[2] - p3 * values[1],
            values[3] * p2 - p0 * values[1] + p1 * values[0] - p3 * values[2] };
    }

    BaseType Dot(const Quaternion& other) const { return static_cast<Vector<BaseType, 4>>(*this).Dot(static_cast<Vector<BaseType, 4>>(other)); }
    BaseType Length() const { return static_cast<Vector<BaseType, 4>>(*this).Length(); }
    BaseType LengthSq() const { return static_cast<Vector<BaseType, 4>>(*this).SquareLength(); }
    Quaternion Normalize() const { return Quaternion<BaseType>(*this) /= Length(); }
    Quaternion Conjugate() const { return Quaternion<BaseType>(values[0] * -1.0f, values[1] * -1.0f, values[2] * -1.0f, values[3]); }
    Quaternion Integrate(Vector<BaseType, 3> const& vector, BaseType scalar)
    {
        Quaternion<BaseType> q(vector[0] * scalar, vector[1] * scalar, vector[2] * scalar, 0.0f);

        q *= *this;

        values[0] += 0.5f * q[0];
        values[1] += 0.5f * q[1];
        values[2] += 0.5f * q[2];
        values[3] += 0.5f * q[3];
    }
    Quaternion Invert() const { return Quaternion<BaseType>(*this).Conjugate() /= LengthSq(); }
    Quaternion Exponent() const
    {
        BaseType a = static_cast<Vector<BaseType, 3>>(*this).Length();
        BaseType c = cosf(a);
        BaseType s = sinf(a) / a;
        return Quaternion<BaseType>(values[0] * s, values[1] * s, values[2] * s, c);
    }
    Quaternion Logarithm() const
    {
        BaseType theta = acos(values[3]);
        BaseType sinTheta = sin(theta);
        theta /= sinTheta;
        return Quaternion<BaseType>(values[0] * theta, values[1] * theta, values[2] * theta, 0);
    }
    Vector<BaseType, 3> RotateVector(Vector<BaseType, 3> const& vector) const
    {
        Quaternion<BaseType> tmp(vector);
        Quaternion<BaseType> conj = Conjugate();
        Quaternion<BaseType> res = Quaternion<BaseType>(*this);
        res *= tmp;
        res *= conj;
        return res.V3();
    }
    Vector<BaseType, 3> EulerAngles() const
    {
        return Vector<BaseType, 3>(atan2(2 * (values[3] * values[0] + values[1] * values[2]), 1 - 2 * (values[0] * values[0] + values[1] * values[1])),
            asin(2 * (values[3] * values[1] - values[2] * values[0])), atan2(2 * (values[3] * values[2] + values[0] * values[1]), 1 - 2 * (values[1] * values[1] + values[2] * values[2])));
    }
    TMatrix4<BaseType> EulerTransform() const
    {
        return TMatrix4<BaseType>(1 - 2 * (values[1] * values[1] + values[2] * values[2]), 2 * (values[0] * values[1] + values[3] * values[2]), 2 * (values[0] * values[2] - values[3] * values[1]), 0,
            2 * (values[0] * values[1] - values[3] * values[2]), 1 - 2 * (values[0] * values[0] + values[2] * values[2]), 2 * (values[1] * values[2] + values[3] * values[0]), 0,
            2 * (values[0] * values[2] + values[3] * values[1]), 2 * (values[1] * values[2] - values[3] * values[0]), 1 - 2 * (values[0] * values[0] + values[1] * values[1]), 0, 0, 0, 0, 1);
    }
    void Zero() { *this = Quaternion(); }
    void Identity()
    {
        values[0] = 0;
        values[1] = 0;
        values[2] = 0;
        values[3] = 1;
    }
    Quaternion GetUnit() const
    {
        BaseType len = Length();
        return Quaternion<BaseType>(values[0] / len, values[1] / len, values[2] / len, values[3] / len);
    }
    Quaternion GetConjugate() const { return Quaternion<BaseType>(-values[0], -values[1], -values[2], values[3]); }
    Quaternion GetInverse() const { return Quaternion<BaseType>(-values[0], -values[1], -values[2], values[3]); }
    TMatrix3<BaseType> GetMatrix() const
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

        return TMatrix3<BaseType>(
            static_cast<BaseType>(1) - yys - zzs, xys - wzs, xzs + wys, xys + wzs, static_cast<BaseType>(1) - xxs - zzs, yzs - wxs, xzs - wys, yzs + wxs, static_cast<BaseType>(1) + xxs - yys);
    }

    static Quaternion Slerp(const Quaternion& src, const Quaternion& dst, BaseType t)
    {
        Quaternion v0 = src.Normalize();
        Quaternion v1 = dst.Normalize();

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
            omega = acosf(dot);  // extract theta from dot product's cos theta
            sinom = sinf(omega);
            sclp = sinf((1.0f - t) * omega) / sinom;
            sclq = sinf(t * omega) / sinom;
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
