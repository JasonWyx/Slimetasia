/******************************************************************************/
/*!
  All content © 2017 DigiPen Institute of Technology Singapore.
  All Rights Reserved

  File Name: Matrix.h
  Author(s): Wang Yuxuan Jason
  Contribution for the file: 100%
  Brief Description:

    This file contains the implementation for the templated Matrix types.

    Note: Matrix types are in column major ordering.
*/
/******************************************************************************/

#pragma once
#include <algorithm>
#include <iostream>

#include "Math.h"
#include "Vector.h"

template <typename T>
class TMatrix2
{
    float m[4];

public:

    explicit TMatrix2()
        : m { 1, 0, 1, 0 }
    {
    }
    explicit TMatrix2(T m0, T m1, T m2, T m3)
        : m { m0, m1, m2, m3 }
    {
    }
    explicit TMatrix2(T val)
        : m { val, 0, val, 0 }
    {
    }

    float GetDeterminant() const;
    TMatrix2& Identity();
    TMatrix2& Invert();
    TMatrix2& Transpose();

    TMatrix2& operator+=(TMatrix2 const& rhs);
    TMatrix2& operator-=(TMatrix2 const& rhs);
    TMatrix2& operator*=(TMatrix2 const& rhs);
    TMatrix2 operator+(TMatrix2 const& rhs) const;
    TMatrix2 operator-(TMatrix2 const& rhs) const;
    TMatrix2 operator*(TMatrix2 const& rhs) const;
    TMatrix2 operator*(T const& rhs) const;
    TMatrix2 operator-() const;
    TVector2<T> operator*(TVector2<T> const& rhs) const;

    T operator[](unsigned index) const;
    T& operator[](unsigned index);

    friend std::ostream& operator<<(std::ostream& os, TMatrix2 const& rhs);

    static TMatrix2<T> Scale(float scaleX, float scaleY);
    static TMatrix2<T> Rotate(float angle);
};

template <typename T>
class TMatrix3
{
    float m[9];

public:

    explicit TMatrix3()
        : m { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
    {
    }
    explicit TMatrix3(T val)
        : m { val, 0, 0, 0, val, 0, 0, 0, val }
    {
    }
    explicit TMatrix3(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8)
        : m { m0, m1, m2, m3, m4, m5, m6, m7, m8 }
    {
    }

    float GetDeterminant() const;
    TMatrix3& Identity();
    TMatrix3& Invert();
    TMatrix3& Transpose();
    void SetAllValues(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8);
    void Zero();
    TMatrix3 GetInverse() const;

    TMatrix3& operator+=(TMatrix3 const& rhs);
    TMatrix3& operator-=(TMatrix3 const& rhs);
    TMatrix3& operator*=(TMatrix3 const& rhs);
    TMatrix3& operator*=(T const& rhs);
    TMatrix3 operator+(TMatrix3 const& rhs) const;
    TMatrix3 operator-(TMatrix3 const& rhs) const;
    TMatrix3 operator*(TMatrix3 const& rhs) const;
    TMatrix3 operator*(T const& rhs) const;
    TMatrix3 operator-() const;
    TVector3<T> operator*(TVector3<T> const& rhs) const;
    T operator[](unsigned index) const;
    T& operator[](unsigned index);

    void SetRow3(unsigned index, TVector3<T> const& v);
    void SetCol3(unsigned index, TVector3<T> const& v);
    TVector3<T> GetRow3(unsigned index) const;
    TVector3<T> GetCol3(unsigned index) const;
    static TMatrix3<T> Scale(float scaleX, float scaleY, float scaleZ);
    static TMatrix3<T> Scale(TVector3<T> const& scale);
    static TMatrix3<T> Rotate(float angleX, float angleY, float angleZ);
    static TMatrix3<T> Rotate(TVector3<T> const& angles);
    static TMatrix3<T> Rotate(TVector3<T> const& axis, TVector3<T> const& angles);
    static TMatrix3<T> RotateX(float angle);
    static TMatrix3<T> RotateY(float angle);
    static TMatrix3<T> RotateZ(float angle);

    friend std::ostream& operator<<(std::ostream& os, TMatrix3 const& rhs);
};

template <typename T>
class TMatrix4
{
    float m[16];

public:

    explicit TMatrix4()
        : m { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
    {
    }
    explicit TMatrix4(T val)
        : m { val, 0, 0, 0, 0, val, 0, 0, 0, 0, val, 0, 0, 0, 0, val }
    {
    }
    explicit TMatrix4(TMatrix3<T> _m)
        : m { _m[0], _m[1], _m[2], 0, _m[3], _m[4], _m[5], 0, _m[6], _m[7], _m[8], 0, 0, 0, 0, 1 }
    {
    }
    explicit TMatrix4(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8, T m9, T m10, T m11, T m12, T m13, T m14, T m15)
        : m { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15 }
    {
    }

    float* GetMatrix() { return m; }
    float GetCoFactor(float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8) const;
    float GetDeterminant() const;
    TMatrix4& Identity();
    TMatrix4& Invert();
    TMatrix4 Inverted() const;
    TMatrix4& Transpose();
    TMatrix4 Transposed() const;

    TMatrix4& operator+=(TMatrix4 const& rhs);
    TMatrix4& operator-=(TMatrix4 const& rhs);
    TMatrix4& operator*=(TMatrix4 const& rhs);
    TMatrix4& operator*=(T scale);
    TMatrix4& operator/=(T scale);
    TMatrix4 operator+(TMatrix4 const& rhs) const;
    TMatrix4 operator-(TMatrix4 const& rhs) const;
    TMatrix4 operator*(TMatrix4 const& rhs) const;
    TMatrix4 operator*(T scale) const;
    TMatrix4 operator/(T scale) const;
    TMatrix4 operator-() const;
    TVector4<T> operator*(TVector4<T> const& rhs) const;
    T operator[](unsigned index) const;
    T& operator[](unsigned index);

    void Decompose(TVector3<T>& translate, TVector3<T>& rotation, TVector3<T>& scale);
    // void        Decompose(TVector3<T>& translate, TQuaternion<T>& orientation, TVector3<T>& scale);

    void SetRow3(unsigned index, TVector3<T> const& v);
    void SetCol3(unsigned index, TVector3<T> const& v);
    void SetRow4(unsigned index, TVector4<T> const& v);
    void SetCol4(unsigned index, TVector4<T> const& v);
    TVector3<T> GetRow3(unsigned index) const;
    TVector4<T> GetRow4(unsigned index) const;
    TVector3<T> GetCol3(unsigned index) const;
    TVector4<T> GetCol4(unsigned index) const;
    TMatrix3<T> ToMat3() const;

    friend std::ostream& operator<<(std::ostream& os, TMatrix4 const& rhs);

    static TMatrix4<T> Translate(TVector3<T> const& v);
    static TMatrix4<T> Rotate(T angle, TVector3<T> const& v);
    static TMatrix4<T> RotateX(T angle);
    static TMatrix4<T> RotateY(T angle);
    static TMatrix4<T> RotateZ(T angle);
    static TMatrix4<T> Scale(TVector3<T> const& v);
    static TMatrix4<T> Scale(T scale);
    static TMatrix4<T> SetFrustumPersp(float l, float r, float b, float t, float n, float f);
    static TMatrix4<T> SetFrustumOrtho(float l, float r, float b, float t, float n, float f);
    static TMatrix4<T> Perspective(float fov, float aspectRatio, float near, float far);
    static TMatrix4<T> LookAt(TVector3<T> const& eye, TVector3<T> const& center, TVector3<T> const& up);
};
/******************************************************************************/
/* TMatrix2<T> Definitions Start */

template <typename T>
float TMatrix2<T>::GetDeterminant() const
{
    return m[0] * m[3] - m[1] * m[2];
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::Identity()
{
    m[0] = 1;
    m[1] = 0;
    m[2] = 1;
    m[3] = 0;
    return *this;
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::Invert()
{
    float determinant = GetDeterminant();
    float invDeterminant = 1.0f / determinant;

    if (fabs(determinant) <= EPSILON) return Identity();

    std::swap(m[0], m[3]);
    m[0] *= invDeterminant;
    m[1] *= -invDeterminant;
    m[2] *= -invDeterminant;
    m[3] *= invDeterminant;

    return *this;
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::Transpose()
{
    std::swap(m[0], m[3]);
    return *this;
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::operator+=(TMatrix2<T> const& rhs)
{
    for (int i = 0; i < sizeof(m); i++)
        m[i] += rhs[i];
    return *this;
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::operator-=(TMatrix2<T> const& rhs)
{
    for (int i = 0; i < sizeof(m); i++)
        m[i] -= rhs[i];
    return *this;
}

template <typename T>
TMatrix2<T>& TMatrix2<T>::operator*=(TMatrix2<T> const& rhs)
{
    *this = *this * rhs;
    return *this;
}

template <typename T>
TMatrix2<T> TMatrix2<T>::operator+(TMatrix2<T> const& rhs) const
{
    return TMatrix2<T>(*this) += rhs;
}

template <typename T>
TMatrix2<T> TMatrix2<T>::operator-(TMatrix2<T> const& rhs) const
{
    return TMatrix2<T>(*this) -= rhs;
}

template <typename T>
TMatrix2<T> TMatrix2<T>::operator*(TMatrix2<T> const& rhs) const
{
    return TMatrix2<T>(m[0] * rhs[0] + m[2] * rhs[1], m[1] * rhs[0] + m[3] * rhs[1], m[0] * rhs[2] + m[2] * rhs[3], m[1] * rhs[2] + m[3] * rhs[3]);
}

template <typename T>
TMatrix2<T> TMatrix2<T>::operator*(T const& rhs) const
{
    return TMatrix2<T> { m[0] * rhs, m[1] * rhs, m[2] * rhs, m[3] * rhs };
}

template <typename T>
TMatrix2<T> TMatrix2<T>::operator-() const
{
    return TMatrix2<T>(-m[0], -m[1], -m[2], -m[3]);
}

template <typename T>
TVector2<T> TMatrix2<T>::operator*(TVector2<T> const& rhs) const
{
    return TVector2<T>(m[0] * rhs.x + m[2] * rhs.y, m[1] * rhs.x + m[3] * rhs.y);
}

template <typename T>
T TMatrix2<T>::operator[](unsigned index) const
{
    return m[index];
}

template <typename T>
T& TMatrix2<T>::operator[](unsigned index)
{
    return m[index];
}

template <typename T>
inline TMatrix2<T> TMatrix2<T>::Scale(float scaleX, float scaleY)
{
    return TMatrix2<T>(scaleX, 0, scaleY, 0);
}

template <typename T>
inline TMatrix2<T> TMatrix2<T>::Rotate(float angle)
{
    return TMatrix2<T>(cosf(angle), sinf(angle), -sinf(angle), cosf(angle));
}

template <typename T>
std::ostream& operator<<(std::ostream& os, TMatrix2<T> const& rhs)
{
    os << "Matrix2[" << rhs[0] << ", " << rhs[1] << ", " << std::endl;
    os << rhs[2] << ", " << rhs[3] << "]" << std::endl;
    return os;
}

/* TMatrix2<T> Definition End */
/******************************************************************************/

/******************************************************************************/
/* TMatrix3<T> Definitions Start */
template <typename T>
float TMatrix3<T>::GetDeterminant() const
{
    return m[0] * m[3] - m[1] * m[2];
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::Identity()
{
    m[0] = 1;
    m[3] = 0;
    m[6] = 0;
    m[1] = 0;
    m[4] = 1;
    m[7] = 0;
    m[2] = 0;
    m[5] = 0;
    m[8] = 1;
    return *this;
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::Invert()
{
    float determinant, invDeterminant;
    float tmp[9];

    tmp[0] = m[4] * m[8] - m[5] * m[7];
    tmp[1] = m[7] * m[2] - m[8] * m[1];
    tmp[2] = m[1] * m[5] - m[2] * m[4];
    tmp[3] = m[5] * m[6] - m[3] * m[8];
    tmp[4] = m[0] * m[8] - m[2] * m[6];
    tmp[5] = m[2] * m[3] - m[0] * m[5];
    tmp[6] = m[3] * m[7] - m[4] * m[6];
    tmp[7] = m[6] * m[1] - m[7] * m[0];
    tmp[8] = m[0] * m[4] - m[1] * m[3];

    // check determinant if it is 0
    determinant = m[0] * tmp[0] + m[1] * tmp[3] + m[2] * tmp[6];
    if (fabs(determinant) <= EPSILON) return Identity();  // cannot inverse, make it idenety matrix

    // divide by the determinant
    invDeterminant = 1.0f / determinant;
    m[0] = invDeterminant * tmp[0];
    m[1] = invDeterminant * tmp[1];
    m[2] = invDeterminant * tmp[2];
    m[3] = invDeterminant * tmp[3];
    m[4] = invDeterminant * tmp[4];
    m[5] = invDeterminant * tmp[5];
    m[6] = invDeterminant * tmp[6];
    m[7] = invDeterminant * tmp[7];
    m[8] = invDeterminant * tmp[8];

    return *this;
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::Transpose()
{
    std::swap(m[1], m[3]);
    std::swap(m[2], m[6]);
    std::swap(m[5], m[7]);
    return *this;
}

template <typename T>
inline void TMatrix3<T>::SetAllValues(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8)
{
    m[0] = m0;
    m[1] = m1;
    m[2] = m2;
    m[3] = m3;
    m[4] = m4;
    m[5] = m5;
    m[6] = m6;
    m[7] = m7;
    m[8] = m8;
}

template <typename T>
void TMatrix3<T>::Zero()
{
    m[0] = static_cast<T>(0);
    m[1] = static_cast<T>(0);
    m[2] = static_cast<T>(0);
    m[3] = static_cast<T>(0);
    m[4] = static_cast<T>(0);
    m[5] = static_cast<T>(0);
    m[6] = static_cast<T>(0);
    m[7] = static_cast<T>(0);
    m[8] = static_cast<T>(0);
}

template <typename T>
TMatrix3<T> TMatrix3<T>::GetInverse() const
{
    TMatrix3 tmp(*this);

    return tmp.Invert();
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::operator+=(TMatrix3<T> const& rhs)
{
    for (int i = 0; i < sizeof(m); i++)
        m[i] += rhs[i];
    return *this;
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::operator-=(TMatrix3<T> const& rhs)
{
    for (int i = 0; i < sizeof(m); i++)
        m[i] -= rhs[i];
    return *this;
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::operator*=(TMatrix3<T> const& rhs)
{
    *this = *this * rhs;
    return *this;
}

template <typename T>
TMatrix3<T>& TMatrix3<T>::operator*=(T const& rhs)
{
    m[0] *= rhs;
    m[1] *= rhs;
    m[2] *= rhs;
    m[3] *= rhs;
    m[4] *= rhs;
    m[5] *= rhs;
    m[6] *= rhs;
    m[7] *= rhs;
    m[8] *= rhs;

    return *this;
}

template <typename T>
TMatrix3<T> TMatrix3<T>::operator+(TMatrix3<T> const& rhs) const
{
    return TMatrix3<T>(*this) += rhs;
}

template <typename T>
TMatrix3<T> TMatrix3<T>::operator-(TMatrix3<T> const& rhs) const
{
    return TMatrix3<T>(*this) -= rhs;
}

template <typename T>
TMatrix3<T> TMatrix3<T>::operator*(TMatrix3<T> const& rhs) const
{
    return TMatrix3<T>(m[0] * rhs[0] + m[3] * rhs[1] + m[6] * rhs[2], m[1] * rhs[0] + m[4] * rhs[1] + m[7] * rhs[2], m[2] * rhs[0] + m[5] * rhs[1] + m[8] * rhs[2],
                       m[0] * rhs[3] + m[3] * rhs[4] + m[6] * rhs[5], m[1] * rhs[3] + m[4] * rhs[4] + m[7] * rhs[5], m[2] * rhs[3] + m[5] * rhs[4] + m[8] * rhs[5],
                       m[0] * rhs[6] + m[3] * rhs[7] + m[6] * rhs[8], m[1] * rhs[6] + m[4] * rhs[7] + m[7] * rhs[8], m[2] * rhs[6] + m[5] * rhs[7] + m[8] * rhs[8]);
}

template <typename T>
TMatrix3<T> TMatrix3<T>::operator*(T const& rhs) const
{
    return TMatrix3<T> { m[0] * rhs, m[1] * rhs, m[2] * rhs, m[3] * rhs, m[4] * rhs, m[5] * rhs, m[6] * rhs, m[7] * rhs, m[8] * rhs };
}

template <typename T>
TMatrix3<T> TMatrix3<T>::operator-() const
{
    return TMatrix3<T>(-m[0], -m[1], -m[2], -m[3], -m[4], -m[5], -m[6], -m[7], -m[8]);
}

template <typename T>
TVector3<T> TMatrix3<T>::operator*(TVector3<T> const& rhs) const
{
    return TVector3<T>(m[0] * rhs.x + m[3] * rhs.y + m[6] * rhs.z, m[1] * rhs.x + m[4] * rhs.y + m[7] * rhs.z, m[2] * rhs.x + m[5] * rhs.y + m[8] * rhs.z);
}

template <typename T>
T TMatrix3<T>::operator[](unsigned index) const
{
    return m[index];
}

template <typename T>
T& TMatrix3<T>::operator[](unsigned index)
{
    return m[index];
}

template <typename T>
inline void TMatrix3<T>::SetRow3(unsigned index, TVector3<T> const& v)
{
    m[index] = v.x;
    m[index + 4] = v.y;
    m[index + 8] = v.z;
}

template <typename T>
inline void TMatrix3<T>::SetCol3(unsigned index, TVector3<T> const& v)
{
    index *= 4;
    m[index] = v.x;
    m[index + 1] = v.y;
    m[index + 2] = v.z;
}

template <typename T>
inline TVector3<T> TMatrix3<T>::GetRow3(unsigned index) const
{
    return TVector3<T>(m[index], m[index + 4], m[index + 8]);
}

template <typename T>
inline TVector3<T> TMatrix3<T>::GetCol3(unsigned index) const
{
    index *= 4;
    return TVector3<T>(m[index], m[index + 1], m[index + 2]);
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::Scale(float scaleX, float scaleY, float scaleZ)
{
    return TMatrix3<T>(scaleX, 0, 0, 0, scaleY, 0, 0, 0, scaleZ);
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::Scale(TVector3<T> const& scale)
{
    return TMatrix3<T>();
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::Rotate(float angleX, float angleY, float angleZ)
{
    return RotateX(angleX) * RotateY(angleY) * RotateZ(angleZ);
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::Rotate(TVector3<T> const& angles)
{
    return RotateX(angles.x) * RotateY(angles.y) * RotateZ(angles.z);
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::Rotate(TVector3<T> const& axis, TVector3<T> const& angles)
{
    return TMatrix3<T>();
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::RotateX(float angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix3<T> tmp(1.0f);

    tmp[4] = c;
    tmp[5] = s;
    tmp[7] = -s;
    tmp[8] = c;

    return tmp;
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::RotateY(float angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix3<T> tmp(1.0f);

    tmp[0] = c;
    tmp[2] = -s;
    tmp[6] = s;
    tmp[8] = c;

    return tmp;
}

template <typename T>
inline TMatrix3<T> TMatrix3<T>::RotateZ(float angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix3<T> tmp(1.0f);

    tmp[0] = c;
    tmp[1] = s;
    tmp[3] = -s;
    tmp[4] = c;

    return tmp;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, TMatrix3<T> const& rhs)
{
    os << "Matrix3[" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ", " << std::endl;
    os << rhs[3] << ", " << rhs[4] << ", " << rhs[5] << ", " << std::endl;
    os << rhs[6] << ", " << rhs[7] << ", " << rhs[8] << "]" << std::endl;
    return os;
}

/* TMatrix3<T> Definition End */
/******************************************************************************/

/******************************************************************************/
/* TMatrix4<T> Definition Begin */

template <typename T>
inline float TMatrix4<T>::GetCoFactor(float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8) const
{
    return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
}

template <typename T>
inline float TMatrix4<T>::GetDeterminant() const
{
    return m[0] * GetCoFactor(m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]) - m[1] * GetCoFactor(m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]) +
           m[2] * GetCoFactor(m[4], m[5], m[7], m[8], m[9], m[11], m[12], m[13], m[15]) - m[3] * GetCoFactor(m[4], m[5], m[6], m[8], m[9], m[10], m[12], m[13], m[14]);
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::Identity()
{
    *this = TMatrix4<T>(1.0f);
    return *this;
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::Invert()
{
    // get cofactors of minor matrices
    float cofactor0 = GetCoFactor(m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]);
    float cofactor1 = GetCoFactor(m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]);
    float cofactor2 = GetCoFactor(m[4], m[5], m[7], m[8], m[9], m[11], m[12], m[13], m[15]);
    float cofactor3 = GetCoFactor(m[4], m[5], m[6], m[8], m[9], m[10], m[12], m[13], m[14]);

    // get determinant
    float determinant = m[0] * cofactor0 - m[1] * cofactor1 + m[2] * cofactor2 - m[3] * cofactor3;
    if (fabs(determinant) <= EPSILON)
    {
        return Identity();
    }

    // get rest of cofactors for adj(M)
    float cofactor4 = GetCoFactor(m[1], m[2], m[3], m[9], m[10], m[11], m[13], m[14], m[15]);
    float cofactor5 = GetCoFactor(m[0], m[2], m[3], m[8], m[10], m[11], m[12], m[14], m[15]);
    float cofactor6 = GetCoFactor(m[0], m[1], m[3], m[8], m[9], m[11], m[12], m[13], m[15]);
    float cofactor7 = GetCoFactor(m[0], m[1], m[2], m[8], m[9], m[10], m[12], m[13], m[14]);

    float cofactor8 = GetCoFactor(m[1], m[2], m[3], m[5], m[6], m[7], m[13], m[14], m[15]);
    float cofactor9 = GetCoFactor(m[0], m[2], m[3], m[4], m[6], m[7], m[12], m[14], m[15]);
    float cofactor10 = GetCoFactor(m[0], m[1], m[3], m[4], m[5], m[7], m[12], m[13], m[15]);
    float cofactor11 = GetCoFactor(m[0], m[1], m[2], m[4], m[5], m[6], m[12], m[13], m[14]);

    float cofactor12 = GetCoFactor(m[1], m[2], m[3], m[5], m[6], m[7], m[9], m[10], m[11]);
    float cofactor13 = GetCoFactor(m[0], m[2], m[3], m[4], m[6], m[7], m[8], m[10], m[11]);
    float cofactor14 = GetCoFactor(m[0], m[1], m[3], m[4], m[5], m[7], m[8], m[9], m[11]);
    float cofactor15 = GetCoFactor(m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]);

    // build inverse matrix = adj(M) / det(M)
    // adjugate of M is the transpose of the cofactor matrix of M
    float invDeterminant = 1.0f / determinant;
    m[0] = invDeterminant * cofactor0;
    m[1] = -invDeterminant * cofactor4;
    m[2] = invDeterminant * cofactor8;
    m[3] = -invDeterminant * cofactor12;

    m[4] = -invDeterminant * cofactor1;
    m[5] = invDeterminant * cofactor5;
    m[6] = -invDeterminant * cofactor9;
    m[7] = invDeterminant * cofactor13;

    m[8] = invDeterminant * cofactor2;
    m[9] = -invDeterminant * cofactor6;
    m[10] = invDeterminant * cofactor10;
    m[11] = -invDeterminant * cofactor14;

    m[12] = -invDeterminant * cofactor3;
    m[13] = invDeterminant * cofactor7;
    m[14] = -invDeterminant * cofactor11;
    m[15] = invDeterminant * cofactor15;

    return *this;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Inverted() const
{
    return TMatrix4<T>(*this).Invert();
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::Transpose()
{
    std::swap(m[1], m[4]);
    std::swap(m[2], m[8]);
    std::swap(m[3], m[12]);
    std::swap(m[6], m[9]);
    std::swap(m[7], m[13]);
    std::swap(m[11], m[14]);

    return *this;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Transposed() const
{
    return TMatrix4(*this).Transpose();
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::operator+=(TMatrix4<T> const& rhs)
{
    m[0] += rhs[0];
    m[1] += rhs[1];
    m[2] += rhs[2];
    m[3] += rhs[3];
    m[4] += rhs[4];
    m[5] += rhs[5];
    m[6] += rhs[6];
    m[7] += rhs[7];
    m[8] += rhs[8];
    m[9] += rhs[9];
    m[10] += rhs[10];
    m[11] += rhs[11];
    m[12] += rhs[12];
    m[13] += rhs[13];
    m[14] += rhs[14];
    m[15] += rhs[15];
    return *this;
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::operator-=(TMatrix4<T> const& rhs)
{
    m[0] -= rhs[0];
    m[1] -= rhs[1];
    m[2] -= rhs[2];
    m[3] -= rhs[3];
    m[4] -= rhs[4];
    m[5] -= rhs[5];
    m[6] -= rhs[6];
    m[7] -= rhs[7];
    m[8] -= rhs[8];
    m[9] -= rhs[9];
    m[10] -= rhs[10];
    m[11] -= rhs[11];
    m[12] -= rhs[12];
    m[13] -= rhs[13];
    m[14] -= rhs[14];
    m[15] -= rhs[15];
    return *this;
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::operator*=(TMatrix4 const& rhs)
{
    TMatrix4<T> tmp = *this;

    m[0] = tmp[0] * rhs[0] + tmp[4] * rhs[1] + tmp[8] * rhs[2] + tmp[12] * rhs[3];
    m[1] = tmp[1] * rhs[0] + tmp[5] * rhs[1] + tmp[9] * rhs[2] + tmp[13] * rhs[3];
    m[2] = tmp[2] * rhs[0] + tmp[6] * rhs[1] + tmp[10] * rhs[2] + tmp[14] * rhs[3];
    m[3] = tmp[3] * rhs[0] + tmp[7] * rhs[1] + tmp[11] * rhs[2] + tmp[15] * rhs[3];
    m[4] = tmp[0] * rhs[4] + tmp[4] * rhs[5] + tmp[8] * rhs[6] + tmp[12] * rhs[7];
    m[5] = tmp[1] * rhs[4] + tmp[5] * rhs[5] + tmp[9] * rhs[6] + tmp[13] * rhs[7];
    m[6] = tmp[2] * rhs[4] + tmp[6] * rhs[5] + tmp[10] * rhs[6] + tmp[14] * rhs[7];
    m[7] = tmp[3] * rhs[4] + tmp[7] * rhs[5] + tmp[11] * rhs[6] + tmp[15] * rhs[7];
    m[8] = tmp[0] * rhs[8] + tmp[4] * rhs[9] + tmp[8] * rhs[10] + tmp[12] * rhs[11];
    m[9] = tmp[1] * rhs[8] + tmp[5] * rhs[9] + tmp[9] * rhs[10] + tmp[13] * rhs[11];
    m[10] = tmp[2] * rhs[8] + tmp[6] * rhs[9] + tmp[10] * rhs[10] + tmp[14] * rhs[11];
    m[11] = tmp[3] * rhs[8] + tmp[7] * rhs[9] + tmp[11] * rhs[10] + tmp[15] * rhs[11];
    m[12] = tmp[0] * rhs[12] + tmp[4] * rhs[13] + tmp[8] * rhs[14] + tmp[12] * rhs[15];
    m[13] = tmp[1] * rhs[12] + tmp[5] * rhs[13] + tmp[9] * rhs[14] + tmp[13] * rhs[15];
    m[14] = tmp[2] * rhs[12] + tmp[6] * rhs[13] + tmp[10] * rhs[14] + tmp[14] * rhs[15];
    m[15] = tmp[3] * rhs[12] + tmp[7] * rhs[13] + tmp[11] * rhs[14] + tmp[15] * rhs[15];

    return *this;
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::operator*=(T scale)
{
    m[0] *= scale;
    m[1] *= scale;
    m[2] *= scale;
    m[3] *= scale;
    m[4] *= scale;
    m[5] *= scale;
    m[6] *= scale;
    m[7] *= scale;
    m[8] *= scale;
    m[9] *= scale;
    m[10] *= scale;
    m[11] *= scale;
    m[12] *= scale;
    m[13] *= scale;
    m[14] *= scale;
    m[15] *= scale;
    return *this;
}

template <typename T>
inline TMatrix4<T>& TMatrix4<T>::operator/=(T scale)
{
    m[0] /= scale;
    m[1] /= scale;
    m[2] /= scale;
    m[3] /= scale;
    m[4] /= scale;
    m[5] /= scale;
    m[6] /= scale;
    m[7] /= scale;
    m[8] /= scale;
    m[9] /= scale;
    m[10] /= scale;
    m[11] /= scale;
    m[12] /= scale;
    m[13] /= scale;
    m[14] /= scale;
    m[15] /= scale;
    return *this;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator+(TMatrix4<T> const& rhs) const
{
    return TMatrix4<T>(*this) += rhs;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator-(TMatrix4<T> const& rhs) const
{
    return TMatrix4<T>(*this) -= rhs;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator*(TMatrix4<T> const& rhs) const
{
    return TMatrix4<T>(*this) *= rhs;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator*(T scale) const
{
    return TMatrix4<T>(*this) *= scale;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator/(T scale) const
{
    return TMatrix4<T>(*this) /= scale;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::operator-() const
{
    return -TMatrix4<T>(*this);
}

template <typename T>
inline TVector4<T> TMatrix4<T>::operator*(TVector4<T> const& rhs) const
{
    return TVector4<T>(m[0] * rhs.x + m[4] * rhs.y + m[8] * rhs.z + m[12] * rhs.w, m[1] * rhs.x + m[5] * rhs.y + m[9] * rhs.z + m[13] * rhs.w,
                       m[2] * rhs.x + m[6] * rhs.y + m[10] * rhs.z + m[14] * rhs.w, m[3] * rhs.x + m[7] * rhs.y + m[11] * rhs.z + m[15] * rhs.w);
}

template <typename T>
inline T TMatrix4<T>::operator[](unsigned index) const
{
    return m[index];
}

template <typename T>
inline T& TMatrix4<T>::operator[](unsigned index)
{
    return m[index];
}

template <typename T>
inline void TMatrix4<T>::Decompose(TVector3<T>& translate, TVector3<T>& rotation, TVector3<T>& scale)
{
    scale[0] = GetCol3(0).Length();
    scale[1] = GetCol3(1).Length();
    scale[2] = GetCol3(2).Length();

    SetCol3(0, GetCol3(0).Normalized());
    SetCol3(1, GetCol3(1).Normalized());
    SetCol3(2, GetCol3(2).Normalized());

    rotation[0] = Math::ToDegrees(atan2f(m[6], m[10]));
    rotation[1] = Math::ToDegrees(atan2f(-m[2], sqrtf(m[6] * m[6] + m[10] * m[10])));
    rotation[2] = Math::ToDegrees(atan2f(m[1], m[0]));

    translate.x = m[12];
    translate.y = m[13];
    translate.z = m[14];
}

template <typename T>
inline void TMatrix4<T>::SetRow3(unsigned index, TVector3<T> const& v)
{
    m[index] = v.x;
    m[index + 4] = v.y;
    m[index + 8] = v.z;
}

template <typename T>
inline void TMatrix4<T>::SetCol3(unsigned index, TVector3<T> const& v)
{
    index *= 4;
    m[index] = v.x;
    m[index + 1] = v.y;
    m[index + 2] = v.z;
}

template <typename T>
inline void TMatrix4<T>::SetRow4(unsigned index, TVector4<T> const& v)
{
    m[index] = v.x;
    m[index + 4] = v.y;
    m[index + 8] = v.z;
    m[index + 12] = v.w;
}

template <typename T>
inline void TMatrix4<T>::SetCol4(unsigned index, TVector4<T> const& v)
{
    index *= 4;
    m[index] = v.x;
    m[index + 1] = v.y;
    m[index + 2] = v.z;
    m[index + 3] = v.w;
}

template <typename T>
inline TVector3<T> TMatrix4<T>::GetRow3(unsigned index) const
{
    return TVector3<T>(m[index], m[index + 4], m[index + 8]);
}

template <typename T>
inline TVector4<T> TMatrix4<T>::GetRow4(unsigned index) const
{
    return TVector3<T>(m[index], m[index + 4], m[index + 8], m[index + 12]);
}

template <typename T>
inline TVector3<T> TMatrix4<T>::GetCol3(unsigned index) const
{
    index *= 4;
    return TVector3<T>(m[index], m[index + 1], m[index + 2]);
}

template <typename T>
inline TVector4<T> TMatrix4<T>::GetCol4(unsigned index) const
{
    index *= 4;
    return TVector3<T>(m[index], m[index + 1], m[index + 2], m[index + 3]);
}

template <typename T>
inline TMatrix3<T> TMatrix4<T>::ToMat3() const
{
    return TMatrix3<T>(m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]);
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Translate(TVector3<T> const& v)
{
    TMatrix4 tmp = TMatrix4();
    tmp[12] = v.x;
    tmp[13] = v.y;
    tmp[14] = v.z;
    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Rotate(T angle, TVector3<T> const& v)
{
    float c = cosf(Math::ToRadians(angle));  // cosine
    float s = sinf(Math::ToRadians(angle));  // sine
    float c1 = 1.0f - c;                     // 1 - c

    // build rotation matrix
    TMatrix4<T> tmp = TMatrix4<T>(v.x * v.x * c1 + c, v.x * v.y * c1 + v.z * s, v.x * v.z * c1 - v.y * s, 0, v.x * v.y * c1 - v.z * s, v.y * v.y * c1 + c, v.y * v.z * c1 + v.x * s, 0,
                                  v.x * v.z * c1 + v.y * s, v.y * v.z * c1 - v.x * s, v.z * v.z * c1 + c, 0, 0, 0, 0, 1);

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::RotateX(T angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix4<T> tmp(1.0f);

    tmp[5] = c;
    tmp[6] = s;
    tmp[9] = -s;
    tmp[10] = c;

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::RotateY(T angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix4<T> tmp(1.0f);

    tmp[0] = c;
    tmp[2] = -s;
    tmp[8] = s;
    tmp[10] = c;

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::RotateZ(T angle)
{
    float c = cosf(Math::ToRadians(angle));
    float s = sinf(Math::ToRadians(angle));

    TMatrix4<T> tmp(1.0f);

    tmp[0] = c;
    tmp[1] = s;
    tmp[4] = -s;
    tmp[5] = c;

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Scale(TVector3<T> const& v)
{
    TMatrix4<T> tmp(1.0);

    tmp[0] = v.x;
    tmp[5] = v.y;
    tmp[10] = v.z;

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Scale(T scale)
{
    TMatrix4<T> tmp(scale);

    tmp[15] = 1.0;

    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::SetFrustumPersp(float l, float r, float b, float t, float n, float f)
{
    TMatrix4<T> tmp(1.0f);
    tmp[0] = 2 * n / (r - l);
    tmp[5] = 2 * n / (t - b);
    tmp[8] = (r + l) / (r - l);
    tmp[9] = (t + b) / (t - b);
    tmp[10] = -(f + n) / (f - n);
    tmp[11] = -1;
    tmp[14] = -(2 * f * n) / (f - n);
    tmp[15] = 0;
    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::SetFrustumOrtho(float l, float r, float b, float t, float n, float f)
{
    TMatrix4<T> tmp;
    tmp[0] = 2 / (r - l);
    tmp[5] = 2 / (t - b);
    tmp[10] = -2 / (f - n);
    tmp[12] = -(r + l) / (r - l);
    tmp[13] = -(t + b) / (t - b);
    tmp[14] = -(f + n) / (f - n);
    return tmp;
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::Perspective(float fov, float aspectRatio, float n, float f)
{
    float t = tanf(Math::ToRadians(fov) / 2);
    float h = n * t;
    float w = h * aspectRatio;

    return TMatrix4<T>::SetFrustumPersp(-w, w, -h, h, n, f);
}

template <typename T>
inline TMatrix4<T> TMatrix4<T>::LookAt(TVector3<T> const& eye, TVector3<T> const& center, TVector3<T> const& worldUp)
{
    TVector3<T> forward = eye - center;
    forward.Normalize();
    TVector3<T> right = worldUp.Cross(forward);
    right.Normalize();
    TVector3<T> up = forward.Cross(right);
    up.Normalize();

    TMatrix4<T> transform;
    transform.SetRow3(0, right);
    transform.SetRow3(1, up);
    transform.SetRow3(2, forward);
    transform *= TMatrix4<T>::Translate(-eye);
    return transform;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, TMatrix4<T> const& rhs)
{
    os << "Matrix4[";
    os << rhs[0] << ", " << rhs[4] << ", " << rhs[8] << ", " << rhs[12] << std::endl;
    os << rhs[1] << ", " << rhs[5] << ", " << rhs[9] << ", " << rhs[13] << std::endl;
    os << rhs[2] << ", " << rhs[6] << ", " << rhs[10] << ", " << rhs[14] << std::endl;
    os << rhs[3] << ", " << rhs[7] << ", " << rhs[11] << ", " << rhs[15] << "]" << std::endl;
    return os;
}
