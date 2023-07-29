#pragma once
#include <limits>

import Math;
import Vector;
import Matrix;
import Quaternion;


// Type definitions
using Vector2 = Vector<float, 2>;
using Vector3 = Vector<float, 3>;
using Vector4 = Vector<float, 4>;
using dVector2 = Vector<double, 2>;
using dVector3 = Vector<double, 3>;
using dVector4 = Vector<double, 4>;
using iVector2 = Vector<int, 2>;
using iVector3 = Vector<int, 3>;
using iVector4 = Vector<int, 4>;
using Quat = Quaternion<float>;
using Quatd = Quaternion<double>;
using Matrix3 = TMatrix3<float>;
using Matrix4 = TMatrix4<float>;
using Color3 = Vector<float, 3>;
using Color4 = Vector<float, 4>;
using UV = Vector<float, 2>;
