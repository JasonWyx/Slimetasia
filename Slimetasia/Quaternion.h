#pragma once
#include "Math.h"
#include "Vector.h"

template <typename T> struct TQuaternion
{
    T x, y, z, w;

    TQuaternion()
        : x(0)
        , y(0)
        , z(0)
        , w(1)
    {
    }
    // Construct from explicit parameters
    explicit TQuaternion(T x, T y, T z, T w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }
    // Construct from vector
    explicit TQuaternion(TVector3<T> const& v)
        : x(v.x)
        , y(v.y)
        , z(v.z)
        , w(0)
    {
    }
    // Construct from axis angle rotation
    explicit TQuaternion(TVector3<T> const& v, T const& angle)
        : x(v.x)
        , y(v.y)
        , z(v.z)
        , w(0)
    {
        T alpha = angle * 0.5f;
        T sinAlpha = sinf(alpha);
        x *= sinAlpha;
        y *= sinAlpha;
        z *= sinAlpha;
        w *= cosf(alpha);
    }

    TQuaternion& operator+=(TQuaternion const& rhs);
    TQuaternion& operator-=(TQuaternion const& rhs);
    TQuaternion& operator*=(TQuaternion const& rhs);
    TQuaternion& operator*=(T rhs);
    TQuaternion& operator/=(T rhs);
    TQuaternion operator-() const;
    TQuaternion operator*(TQuaternion const& rhs) const;
    TQuaternion operator+(TQuaternion const& rhs) const;
    TQuaternion operator-(TQuaternion const& rhs) const;
    TQuaternion operator*(T const& rhs) const;
    TQuaternion operator/(T const& rhs) const;
    TVector3<T> operator*(const TVector3<T>& rhs) const;
    bool operator==(TQuaternion const& rhs) const;
    bool operator!=(TQuaternion const& rhs) const;

    T Dot(TQuaternion const& rhs) const;
    T Length() const;
    T LengthSq() const;
    TQuaternion Normalize() const;
    TQuaternion Conjugate() const;
    TQuaternion Integrate(TVector3<T> const& vector, T scalar);
    TQuaternion Invert() const;
    TQuaternion Exponent() const;
    TQuaternion Logarithm() const;
    TVector3<T> RotateVector(TVector3<T> const& vector) const;
    TVector3<T> EulerAngles() const;
    TMatrix4<T> EulerTransform() const;
    void Zero();
    void Identity();
    TQuaternion GetUnit() const;
    TQuaternion GetConjugate() const;
    TQuaternion GetInverse() const;
    TMatrix3<T> GetMatrix() const;

    TVector3<T> V3() const;
    TVector4<T> V4() const;

    static TQuaternion Slerp(TQuaternion const& src, TQuaternion const& dst, float t);
};

template <typename T> inline TQuaternion<T>& TQuaternion<T>::operator+=(TQuaternion<T> const& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

template <typename T> inline TQuaternion<T>& TQuaternion<T>::operator-=(TQuaternion<T> const& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

template <typename T> inline TQuaternion<T>& TQuaternion<T>::operator*=(TQuaternion<T> const& rhs)
{
    TQuaternion<T> lhs(*this);
    x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
    y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
    z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;
    w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
    return *this;
}

template <typename T> inline TQuaternion<T>& TQuaternion<T>::operator*=(T rhs)
{
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
}

template <typename T> inline TQuaternion<T>& TQuaternion<T>::operator/=(T rhs)
{
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator-() const
{
    return TQuaternion(-x, -y, -z, -w);
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator*(TQuaternion<T> const& rhs) const
{
    return TQuaternion<T>(*this) *= rhs;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator+(TQuaternion<T> const& rhs) const
{
    return TQuaternion<T>(*this) += rhs;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator-(TQuaternion<T> const& rhs) const
{
    return TQuaternion<T>(*this) -= rhs;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator*(T const& rhs) const
{
    return TQuaternion<T>(*this) *= rhs;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::operator/(T const& rhs) const
{
    return TQuaternion<T>(*this) /= rhs;
}

template <typename T> inline TVector3<T> TQuaternion<T>::operator*(const TVector3<T>& rhs) const
{
    const float productX = w * rhs.x + y * rhs.z - z * rhs.y;
    const float productY = w * rhs.y + z * rhs.x - x * rhs.z;
    const float productZ = w * rhs.z + x * rhs.y - y * rhs.x;
    const float productW = -x * rhs.x - y * rhs.y - z * rhs.z;
    return TVector3<T>(w * productX - productY * z + productZ * y - productW * x, w * productY - productZ * x + productX * z - productW * y, w * productZ - productX * y + productY * x - productW * z);
}

template <typename T> inline bool TQuaternion<T>::operator==(TQuaternion<T> const& rhs) const
{
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

template <typename T> inline bool TQuaternion<T>::operator!=(TQuaternion<T> const& rhs) const
{
    return !(*this == rhs);
}

template <typename T> inline T TQuaternion<T>::Dot(TQuaternion<T> const& rhs) const
{
    return V4().Dot(rhs.V4());
}

template <typename T> inline T TQuaternion<T>::Length() const
{
    return V4().Length();
}

template <typename T> inline T TQuaternion<T>::LengthSq() const
{
    return V4().LengthSq();
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Normalize() const
{
    return TQuaternion<T>(*this) /= Length();
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Conjugate() const
{
    return TQuaternion<T>(x * -1.0f, y * -1.0f, z * -1.0f, w);
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Integrate(TVector3<T> const& vector, T scalar)
{
    TQuaternion<T> q(vector.x * scalar, vector.y * scalar, vector.z * scalar, 0.0f);

    q *= *this;

    x += 0.5f * q.x;
    y += 0.5f * q.y;
    z += 0.5f * q.z;
    w += 0.5f * q.w;
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Invert() const
{
    return TQuaternion<T>(*this).Conjugate() /= LengthSq();
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Exponent() const
{
    T a = V3().Length();
    T c = cosf(a);
    T s = sinf(a) / a;
    return TQuaternion<T>(x * s, y * s, z * s, c);
}

template <typename T> inline TQuaternion<T> TQuaternion<T>::Logarithm() const
{
    T theta = acos(w);
    T sinTheta = sin(theta);
    theta /= sinTheta;
    return TQuaternion<T>(x * theta, y * theta, z * theta, 0);
}

template <typename T> inline TVector3<T> TQuaternion<T>::RotateVector(TVector3<T> const& vector) const
{
    TQuaternion<T> tmp(vector);
    TQuaternion<T> conj = Conjugate();
    TQuaternion<T> res = TQuaternion<T>(*this);
    res *= tmp;
    res *= conj;
    return res.V3();
}

template <typename T> inline TVector3<T> TQuaternion<T>::EulerAngles() const
{
    return TVector3<T>(atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y)), asin(2 * (w * y - z * x)), atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z)));
}

template <typename T> inline TMatrix4<T> TQuaternion<T>::EulerTransform() const
{
    return TMatrix4<T>(1 - 2 * (y * y + z * z), 2 * (x * y + w * z), 2 * (x * z - w * y), 0, 2 * (x * y - w * z), 1 - 2 * (x * x + z * z), 2 * (y * z + w * x), 0, 2 * (x * z + w * y),
                       2 * (y * z - w * x), 1 - 2 * (x * x + y * y), 0, 0, 0, 0, 1);
}

template <typename T> inline void TQuaternion<T>::Zero()
{
    *this = TQuaternion();
}

template <typename T> void TQuaternion<T>::Identity()
{
    x = 0;
    y = 0;
    z = 0;
    w = 1;
}

template <typename T> TQuaternion<T> TQuaternion<T>::GetUnit() const
{
    float len = Length();
    return TQuaternion<T>(x / len, y / len, z / len, w / len);
}

template <typename T> TQuaternion<T> TQuaternion<T>::GetConjugate() const
{
    return TQuaternion<T>(-x, -y, -z, w);
}

template <typename T> TQuaternion<T> TQuaternion<T>::GetInverse() const
{
    return TQuaternion<T>(-x, -y, -z, w);
}

template <typename T> inline TMatrix3<T> TQuaternion<T>::GetMatrix() const
{
    T nQ = x * x + y * y + z * z + w * w;
    T s = nQ > static_cast<T>(0) ? (static_cast<T>(2) / nQ) : static_cast<T>(0);

    T xs = x * s;
    T ys = y * s;
    T zs = z * s;
    T wxs = w * xs;
    T wys = w * ys;
    T wzs = w * zs;
    T xxs = x * xs;
    T xys = x * ys;
    T xzs = x * zs;
    T yys = y * ys;
    T yzs = y * zs;
    T zys = z * ys;
    T zzs = z * zs;

    return TMatrix3<T>(static_cast<T>(1) - yys - zzs, xys - wzs, xzs + wys, xys + wzs, static_cast<T>(1) - xxs - zzs, yzs - wxs, xzs - wys, yzs + wxs, static_cast<T>(1) + xxs - yys);
}

template <typename T> inline TVector3<T> TQuaternion<T>::V3() const
{
    return TVector3<T>(x, y, z);
}

template <typename T> inline TVector4<T> TQuaternion<T>::V4() const
{
    return TVector4<T>(x, y, z, w);
}

template <typename T> TQuaternion<T> TQuaternion<T>::Slerp(TQuaternion<T> const& src, TQuaternion<T> const& dst, float t)
{
    TQuaternion<T> v0 = src.Normalize();
    TQuaternion<T> v1 = dst.Normalize();

    float dot = v0.Dot(v1);

    if (dot < 0)
    {
        v1 = -v1;
        dot = -dot;
    }

    // Calculate coefficients
    float sclp, sclq;
    if ((1.0f - dot) > 0.0001f)  // 0.0001 -> some epsillon
    {
        // Standard case (slerp)
        float omega, sinom;
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
