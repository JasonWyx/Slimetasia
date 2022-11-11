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

#pragma once

#include <cmath>
#include <iostream>

#include "Math.h"

template <typename T>
struct TVector2
{
    union
    {
        struct
        {
            T x;
            T y;
        };
        struct
        {
            T u;
            T v;
        };
        T data[2];
    };

    // 2 Constructors to prevent implicit conversion
    explicit TVector2()
        : x(0)
        , y(0) {};
    explicit TVector2(T splat)
        : x(splat)
        , y(splat)
    {
    }
    explicit TVector2(T _x, T _y)
        : x(_x)
        , y(_y)
    {
    }

    // Operators
    TVector2 operator-() const;                     // Negation
    TVector2& operator+=(TVector2 const& rhs);      // Component-wise addition
    TVector2& operator-=(TVector2 const& rhs);      // Component-wise subtraction
    TVector2& operator*=(TVector2 const& rhs);      // Component-wise multiplication
    TVector2& operator/=(TVector2 const& rhs);      // Component-wise division
    TVector2& operator*=(float const& scalar);      // Scaling (Multiply)
    TVector2& operator/=(float const& scalar);      // Scaling (Division)
    TVector2 operator+(TVector2 const& rhs) const;  // Component-wise addition
    TVector2 operator-(TVector2 const& rhs) const;  // Component-wise subtraction
    TVector2 operator*(TVector2 const& rhs) const;  // Component-wise multiplication
    TVector2 operator/(TVector2 const& rhs) const;  // Component-wise division
    TVector2 operator*(float const& scalar) const;  // Scaling (Multiply)
    TVector2 operator/(float const& scalar) const;  // Scaling (Division)
    T& operator[](unsigned index);                  // Indexing Mutable
    T operator[](unsigned index) const;             // Indexing Immutable
    bool operator!=(TVector2 const& rhs) const;
    bool operator==(TVector2 const& rhs) const;  // Comparison

    // Utilities
    TVector2 Normalize();
    TVector2 Projection(TVector2 const& rhs);
    float Dot(TVector2 const& rhs) const;
    float Angle() const;
    TVector2 Rotate(float angle) const;
    float Length() const;
    float SquareLength() const;
    float Distance(TVector2 const& rhs) const;
    float SquareDistance(TVector2 const& rhs) const;
    void Zero();
};

template <typename T>
struct TVector3
{
    union
    {
        struct
        {
            T x;
            T y;
            T z;
        };
        struct
        {
            T r;
            T g;
            T b;
        };
        struct
        {
            T u;
            T v;
            T s;
        };
        T data[3];
    };

    // Constructors
    explicit TVector3()
        : x(0)
        , y(0)
        , z(0) {};
    explicit TVector3(T splat)
        : x(splat)
        , y(splat)
        , z(splat)
    {
    }
    explicit TVector3(T _x, T _y, T _z)
        : x(_x)
        , y(_y)
        , z(_z)
    {
    }
    explicit TVector3(TVector2<T> v, T _z)
        : x(v.x)
        , y(v.y)
        , z(_z)
    {
    }

    // Operators
    TVector3 operator-() const;                     // Negation
    TVector3& operator+=(TVector3 const& rhs);      // Component-wise addition
    TVector3& operator-=(TVector3 const& rhs);      // Component-wise subtraction
    TVector3& operator*=(TVector3 const& rhs);      // Component-wise multiplication
    TVector3& operator/=(TVector3 const& rhs);      // Component-wise division
    TVector3& operator*=(float const& scalar);      // Scaling (Multiply)
    TVector3& operator/=(float const& scalar);      // Scaling (Division)
    TVector3 operator+(TVector3 const& rhs) const;  // Component-wise addition
    TVector3 operator-(TVector3 const& rhs) const;  // Component-wise subtraction
    TVector3 operator*(TVector3 const& rhs) const;  // Component-wise multiplication
    TVector3 operator/(TVector3 const& rhs) const;  // Component-wise division
    TVector3 operator*(float const& scalar) const;  // Scaling (Multiply)
    TVector3 operator/(float const& scalar) const;  // Scaling (Division)
    T& operator[](unsigned index);                  // Indexing Mutable
    T operator[](unsigned index) const;             // Indexing Immutable
    bool operator!=(TVector3 const& rhs) const;
    bool operator==(TVector3 const& rhs) const;  // Comparison
    bool operator<(TVector3 const& rhs) const;   // Comparison(x & y & z are > rhs x & y & z)
    bool operator<=(TVector3 const& rhs) const;  // Comparison(x & y & z are >=  rhs x & y & z)
    bool operator>(TVector3 const& rhs) const;   // Comparison(x & y & z are < rhs x & y & z)
    bool operator>=(TVector3 const& rhs) const;  // Comparison(x & y & z are <= rhs x & y & z)

    // Utilities
    TVector3& Normalize();
    TVector3 Normalized() const;
    TVector3 Projection(TVector3 const& rhs);
    TVector3 Cross(TVector3 const& rhs) const;
    float Dot(TVector3 const& rhs) const;
    float Length() const;
    float SquareLength() const;
    float Distance(TVector3 const& rhs) const;
    float SquareDistance(TVector3 const& rhs) const;
    void Zero();
    TVector2<T> PolarAngles() const;
    TVector3 RotateX(float const& angle);
    TVector3 RotateY(float const& angle);
    TVector3 RotateZ(float const& angle);
    TVector3 OrthogonalUnitVector() const;
    float* GetVector() { return data; }
};

template <typename T>
struct TVector4
{
    union
    {
        struct
        {
            T x;
            T y;
            T z;
            T w;
        };
        struct
        {
            T r;
            T g;
            T b;
            T a;
        };
        T data[4];
    };

    // 4 Constructors to prevent implicit conversion
    explicit TVector4()
        : x(0)
        , y(0)
        , z(0)
        , w(0) {};
    explicit TVector4(T splat)
        : x(splat)
        , y(splat)
        , z(splat)
        , w(splat)
    {
    }
    explicit TVector4(T _x, T _y, T _z, T _w)
        : x(_x)
        , y(_y)
        , z(_z)
        , w(_w)
    {
    }
    explicit TVector4(TVector2<T> v, T _z, T _w)
        : x(v.x)
        , y(v.y)
        , z(_z)
        , w(_w)
    {
    }
    explicit TVector4(TVector3<T> v, T _w)
        : x(v.x)
        , y(v.y)
        , z(v.z)
        , w(_w)
    {
    }

    // Operators
    TVector4 operator-() const;                     // Negation
    TVector4& operator+=(TVector4 const& rhs);      // Component-wise addition
    TVector4& operator-=(TVector4 const& rhs);      // Component-wise subtraction
    TVector4& operator*=(TVector4 const& rhs);      // Component-wise multiplication
    TVector4& operator/=(TVector4 const& rhs);      // Component-wise division
    TVector4& operator*=(float const& scalar);      // Scaling (Multiply)
    TVector4& operator/=(float const& scalar);      // Scaling (Division)
    TVector4 operator+(TVector4 const& rhs) const;  // Component-wise addition
    TVector4 operator-(TVector4 const& rhs) const;  // Component-wise subtraction
    TVector4 operator*(TVector4 const& rhs) const;  // Component-wise multiplication
    TVector4 operator/(TVector4 const& rhs) const;  // Component-wise division
    TVector4 operator*(float const& scalar) const;  // Scaling (Multiply)
    TVector4 operator/(float const& scalar) const;  // Scaling (Division)
    T& operator[](unsigned index);                  // Indexing Mutable
    T operator[](unsigned index) const;             // Indexing Immutable
    bool operator!=(TVector4 const& rhs) const;     // Comparison
    bool operator==(TVector4 const& rhs) const;     // Comparison

    // Utilities
    TVector4 Normalize();
    float Dot(TVector4 const& rhs) const;
    float Length() const;
    float LengthSq() const;
    float Distance(TVector4 const& rhs) const;
    float SquareDistance(TVector4 const& rhs) const;
    void Zero();

    TVector3<T> V3();
};

/******************************************************************************/
/* TVector2 Definitions Start */

// Negates vector.
template <typename T>
inline TVector2<T> TVector2<T>::operator-() const
{
    return TVector2<T>(-x, -y);
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator+=(TVector2<T> const& rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator-=(TVector2<T> const& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator*=(TVector2<T> const& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    return *this;
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator/=(TVector2<T> const& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    return *this;
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator*=(float const& scalar)
{
    x = static_cast<T>(static_cast<float>(x) * scalar);
    y = static_cast<T>(static_cast<float>(y) * scalar);
    return *this;
}

template <typename T>
inline TVector2<T>& TVector2<T>::operator/=(float const& scalar)
{
    x = static_cast<T>(static_cast<float>(x) / scalar);
    y = static_cast<T>(static_cast<float>(y) / scalar);
    return *this;
}

template <typename T>
inline TVector2<T> TVector2<T>::operator+(TVector2<T> const& rhs) const
{
    return TVector2<T>(*this) += rhs;
}

template <typename T>
inline TVector2<T> TVector2<T>::operator-(TVector2<T> const& rhs) const
{
    return TVector2<T>(*this) -= rhs;
}

template <typename T>
inline TVector2<T> TVector2<T>::operator*(float const& scalar) const
{
    return TVector2<T>(*this) *= scalar;
}

template <typename T>
inline TVector2<T> TVector2<T>::operator/(float const& scalar) const
{
    return TVector2<T>(*this) /= scalar;
}

template <typename T>
inline T& TVector2<T>::operator[](unsigned index)
{
    return data[index];
}

template <typename T>
inline T TVector2<T>::operator[](unsigned index) const
{
    return data[index];
}

template <typename T>
inline bool TVector2<T>::operator!=(TVector2 const& rhs) const
{
    return x != rhs.x || y != rhs.y;
}

template <typename T>
inline bool TVector2<T>::operator==(TVector2 const& rhs) const
{
    return x == rhs.x && y == rhs.y;
}

template <typename T>
inline TVector2<T> TVector2<T>::Normalize()
{
    float length = Length();
    return TVector2<T>(*this) / length;
}

template <typename T>
inline TVector2<T> TVector2<T>::Projection(TVector2<T> const& rhs)
{
    TVector2<T> tmp = rhs;
    tmp.Normalize();
    return tmp *= this->Dot(tmp);
}

template <typename T>
inline float TVector2<T>::Dot(TVector2<T> const& rhs) const
{
    return (x * rhs.x + y * rhs.y);
}

template <typename T>
inline float TVector2<T>::Angle() const
{
    return atan2f(-y, -x) * RAD_TO_DEG + 180;
}

template <typename T>
inline TVector2<T> TVector2<T>::Rotate(float angle) const
{
    angle = DEG_TO_RAD * angle;
    return TVector2<T>(x * cosf(angle) - y * sinf(angle), x * sinf(angle) + y * cosf(angle));
}

template <typename T>
inline TVector2<T> TVector2<T>::operator*(TVector2<T> const& rhs) const
{
    return TVector2<T>(x * rhs.x, y * rhs.y);
}

template <typename T>
inline TVector2<T> TVector2<T>::operator/(TVector2<T> const& rhs) const
{
    return TVector2<T>(x / rhs.x, y / rhs.y);
}

template <typename T>
inline float TVector2<T>::Length() const
{
    return sqrtf(SquareLength());
}

template <typename T>
inline float TVector2<T>::SquareLength() const
{
    return this->Dot(*this);
}

template <typename T>
inline float TVector2<T>::Distance(TVector2<T> const& rhs) const
{
    return sqrtf(SquareDistance(rhs));
}

template <typename T>
inline float TVector2<T>::SquareDistance(TVector2<T> const& rhs) const
{
    TVector2<T> tmp(rhs - *this);
    return tmp.SquareLength();
}

template <typename T>
inline void TVector2<T>::Zero()
{
    x = 0.f;
    y = 0.f;
}

template <typename T>
inline TVector2<T> operator*(float scalar, TVector2<T> const& rhs)
{
    return rhs * scalar;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, TVector2<T> const& rhs)
{
    os << "Vector2[" << rhs.x << "," << rhs.y << "]" << std::endl;
    return os;
}

/* TVector2 Definition End */
/******************************************************************************/

/******************************************************************************/
/* TVector3 Definition Start */

template <typename T>
inline TVector3<T> TVector3<T>::operator-() const
{
    return TVector3<T>(-x, -y, -z);
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator+=(TVector3<T> const& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator-=(TVector3<T> const& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator*=(TVector3<T> const& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator/=(TVector3<T> const& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator*=(float const& scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

template <typename T>
inline TVector3<T>& TVector3<T>::operator/=(float const& scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

template <typename T>
inline TVector3<T> TVector3<T>::operator+(TVector3<T> const& rhs) const
{
    return TVector3<T>(*this) += rhs;
}

template <typename T>
inline TVector3<T> TVector3<T>::operator-(TVector3<T> const& rhs) const
{
    return TVector3<T>(*this) -= rhs;
}

template <typename T>
inline TVector3<T> TVector3<T>::operator*(float const& scalar) const
{
    return TVector3<T>(*this) *= scalar;
}

template <typename T>
inline TVector3<T> TVector3<T>::operator/(float const& scalar) const
{
    return TVector3<T>(*this) /= scalar;
}

template <typename T>
inline T& TVector3<T>::operator[](unsigned index)
{
    return data[index];
}

template <typename T>
inline T TVector3<T>::operator[](unsigned index) const
{
    return data[index];
}

template <typename T>
inline bool TVector3<T>::operator!=(TVector3 const& rhs) const
{
    return x != rhs.x || y != rhs.y || z != rhs.z;
}

template <typename T>
inline bool TVector3<T>::operator==(TVector3 const& rhs) const
{
    return (x == rhs.x && y == rhs.y && z == rhs.z);
}

template <typename T>
inline bool TVector3<T>::operator<=(TVector3 const& rhs) const
{
    return (x <= rhs.x && y <= rhs.y && z <= rhs.z);
}

template <typename T>
inline bool TVector3<T>::operator<(TVector3 const& rhs) const
{
    return (x < rhs.x && y < rhs.y && z < rhs.z);
}

template <typename T>
inline bool TVector3<T>::operator>(TVector3 const& rhs) const
{
    return (x > rhs.x && y > rhs.y && z > rhs.z);
}

template <typename T>
inline bool TVector3<T>::operator>=(TVector3 const& rhs) const
{
    return (x >= rhs.x && y >= rhs.y && z >= rhs.z);
}

template <typename T>
inline TVector3<T>& TVector3<T>::Normalize()
{
    auto len = Length();

    if (len > EPSILON)
    {
        return *this /= Length();
    }
    else
    {
        Zero();
        return *this;
    }
}

template <typename T>
inline TVector3<T> TVector3<T>::Normalized() const
{
    return TVector3<T>(*this) / Length();
}

template <typename T>
inline TVector3<T> TVector3<T>::Projection(TVector3<T> const& rhs)
{
    TVector3<T> tmp = rhs;
    tmp.Normalize();
    return tmp * this->Dot(tmp);
}

template <typename T>
inline TVector3<T> TVector3<T>::Cross(TVector3<T> const& rhs) const
{
    return TVector3<T>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

template <typename T>
inline float TVector3<T>::Dot(TVector3<T> const& rhs) const
{
    return (x * rhs.x + y * rhs.y + z * rhs.z);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator*(TVector3<T> const& rhs) const
{
    return TVector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
}

template <typename T>
inline TVector3<T> TVector3<T>::operator/(TVector3<T> const& rhs) const
{
    return TVector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
}

template <typename T>
inline float TVector3<T>::Length() const
{
    return sqrtf(SquareLength());
}

template <typename T>
inline float TVector3<T>::SquareLength() const
{
    return this->Dot(*this);
}

template <typename T>
inline float TVector3<T>::Distance(TVector3<T> const& rhs) const
{
    return sqrtf(SquareDistance(rhs));
}

template <typename T>
inline float TVector3<T>::SquareDistance(TVector3<T> const& rhs) const
{
    TVector3<T> tmp(rhs - *this);
    return tmp.Dot(tmp);
}

template <typename T>
inline void TVector3<T>::Zero()
{
    x = 0.f;
    y = 0.f;
    z = 0.f;
}

template <typename T>
inline TVector2<T> TVector3<T>::PolarAngles() const
{
    TVector3<T> tmp = this->Normalized();
    TVector2<T> result;

    result.x = Math::ToDegrees(acosf(tmp.y));
    result.y = Math::ToDegrees(atan2f(-tmp.z, tmp.x));

    return result;
}

template <typename T>
inline TVector3<T> TVector3<T>::RotateX(float const& angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    return TVector3<T>(x, y * c + -s * z, s * y + c * z);
}

template <typename T>
inline TVector3<T> TVector3<T>::RotateY(float const& angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    return TVector3<T>(c * x + s * z, y, -s * x + c * z);
}

template <typename T>
inline TVector3<T> TVector3<T>::RotateZ(float const& angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    return TVector3<T>(c * x + -s * y, s * x + c * y, z);
}

template <typename T>
inline TVector3<T> TVector3<T>::OrthogonalUnitVector() const
{
    // Get the minimum element of the vector
    TVector3<T> vectorAbs(std::fabs(x), std::fabs(y), std::fabs(z));
    T minElement = vectorAbs.x < vectorAbs.y && vectorAbs.x < vectorAbs.z ? vectorAbs.x : vectorAbs.y < vectorAbs.z ? vectorAbs.y : vectorAbs.z;

    if (minElement == 0) return TVector3<T>(0.0, -z, y) / std::sqrt(y * y + z * z);

    if (minElement == 1) return TVector3<T>(-z, 0.0, x) / std::sqrt(x * x + z * z);

    return TVector3<T>(-y, x, 0.0) / std::sqrt(x * x + y * y);
}

template <typename T>
inline TVector3<T> operator*(float scalar, TVector3<T> const& rhs)
{
    return rhs * scalar;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, TVector3<T> const& rhs)
{
    os << "Vector3[" << rhs.x << "," << rhs.y << "," << rhs.z << "]" << std::endl;
    return os;
}

/* TVector3 Definition End */
/******************************************************************************/

/******************************************************************************/
/* TVector4 Definition Start */

template <typename T>
inline TVector4<T> TVector4<T>::operator-() const
{
    return TVector4<T>(-x, -y, -z, -w);
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator+=(TVector4<T> const& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator-=(TVector4<T> const& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator*=(TVector4<T> const& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator/=(TVector4<T> const& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
    return *this;
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator*=(float const& scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

template <typename T>
inline TVector4<T>& TVector4<T>::operator/=(float const& scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

template <typename T>
inline TVector4<T> TVector4<T>::operator+(TVector4<T> const& rhs) const
{
    return TVector4<T>(*this) += rhs;
}

template <typename T>
inline TVector4<T> TVector4<T>::operator-(TVector4<T> const& rhs) const
{
    return TVector4<T>(*this) -= rhs;
}

template <typename T>
inline TVector4<T> TVector4<T>::operator*(float const& scalar) const
{
    return TVector4<T>(*this) *= scalar;
}

template <typename T>
inline TVector4<T> TVector4<T>::operator/(float const& scalar) const
{
    return TVector4<T>(*this) /= scalar;
}

template <typename T>
inline T& TVector4<T>::operator[](unsigned index)
{
    return data[index];
}

template <typename T>
inline T TVector4<T>::operator[](unsigned index) const
{
    return data[index];
}

template <typename T>
inline bool TVector4<T>::operator!=(TVector4 const& rhs) const
{
    return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
}

template <typename T>
inline bool TVector4<T>::operator==(TVector4 const& rhs) const
{
    return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
}

template <typename T>
inline TVector4<T> TVector4<T>::operator*(TVector4<T> const& rhs) const
{
    return TVector4<T>(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}

template <typename T>
inline TVector4<T> TVector4<T>::operator/(TVector4<T> const& rhs) const
{
    return TVector4<T>(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}

template <typename T>
inline TVector4<T> TVector4<T>::Normalize()
{
    float length = Length();
    return TVector4<T>(*this) / length;
}

template <typename T>
inline float TVector4<T>::Dot(TVector4<T> const& rhs) const
{
    return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
}

template <typename T>
inline float TVector4<T>::Length() const
{
    return sqrtf(LengthSq());
}

template <typename T>
inline float TVector4<T>::LengthSq() const
{
    return this->Dot(*this);
}

template <typename T>
inline float TVector4<T>::Distance(TVector4<T> const& rhs) const
{
    return sqrtf(SquareDistance(rhs));
}

template <typename T>
inline float TVector4<T>::SquareDistance(TVector4<T> const& rhs) const
{
    return this->Dot(*this);
}

template <typename T>
inline void TVector4<T>::Zero()
{
    x = 0.f;
    y = 0.f;
    z = 0.f;
}

template <typename T>
inline TVector3<T> TVector4<T>::V3()
{
    return TVector3<T>(x, y, z);
}

template <typename T>
inline TVector4<T> operator*(float scalar, TVector4<T> const& rhs)
{
    return rhs * scalar;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, TVector4<T> const& rhs)
{
    os << "Vector4[" << rhs.x << "," << rhs.y << "," << rhs.z << "," << rhs.w << "]" << std::endl;
    return os;
}

/* TVector4 Definition End */
/******************************************************************************/
