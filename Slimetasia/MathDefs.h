#pragma once
#include <limits>

#include "Math.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Vector.h"

char constexpr MAX_CHAR = (std::numeric_limits<char>::max)();
char constexpr MIN_CHAR = (std::numeric_limits<char>::min)();
unsigned char constexpr MAX_UCHAR = (std::numeric_limits<unsigned char>::max)();
unsigned char constexpr MIN_UCHAR = (std::numeric_limits<unsigned char>::min)();
short constexpr MAX_SHORT = (std::numeric_limits<short>::max)();
short constexpr MIN_SHORT = (std::numeric_limits<short>::min)();
unsigned short constexpr MAX_USHORT = (std::numeric_limits<unsigned short>::max)();
unsigned short constexpr MIN_USHORT = (std::numeric_limits<unsigned short>::min)();
long constexpr MAX_LONG = (std::numeric_limits<long>::max)();
long constexpr MIN_LONG = (std::numeric_limits<long>::min)();
unsigned long constexpr MAX_ULONG = (std::numeric_limits<unsigned long>::max)();
unsigned long constexpr MIN_ULONG = (std::numeric_limits<unsigned long>::min)();
long long constexpr MAX_LONGLONG = (std::numeric_limits<long long>::max)();
long long constexpr MIN_LONGLONG = (std::numeric_limits<long long>::max)();
unsigned long long constexpr MAX_ULONGLONG = (std::numeric_limits<unsigned long long>::max)();
unsigned long long constexpr MIN_ULONGLONG = (std::numeric_limits<unsigned long long>::min)();
int constexpr MAX_INT = (std::numeric_limits<int>::max)();
int constexpr MIN_INT = (std::numeric_limits<int>::min)();
unsigned int constexpr MAX_UINT = (std::numeric_limits<unsigned int>::max)();
unsigned int constexpr MIN_UINT = (std::numeric_limits<unsigned int>::min)();
float constexpr MAX_FLOAT = (std::numeric_limits<float>::max)();
float constexpr MIN_FLOAT = (std::numeric_limits<float>::min)();
double constexpr MAX_DOUBLE = (std::numeric_limits<double>::max)();
double constexpr MIN_DOUBLE = (std::numeric_limits<double>::min)();
long double constexpr MAX_LONGDOUBLE = (std::numeric_limits<long double>::max)();
long double constexpr MIN_LONGDOUBLE = (std::numeric_limits<long double>::min)();

// Type definitions
using Vector2 = TVector2<float>;
using Vector3 = TVector3<float>;
using Vector4 = TVector4<float>;
using dVector2 = TVector2<double>;
using dVector3 = TVector3<double>;
using dVector4 = TVector4<double>;
using iVector2 = TVector2<int>;
using iVector3 = TVector3<int>;
using iVector4 = TVector4<int>;
using Quaternion = TQuaternion<float>;
using dQuat = TQuaternion<double>;
using Matrix3 = TMatrix3<float>;
using Matrix4 = TMatrix4<float>;
using Color3 = TVector3<float>;
using Color4 = TVector4<float>;
using UV = TVector2<float>;
