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

module;

#include <algorithm>
#include <iostream>
#include <limits>

export module Matrix;

import Math;
import Vector;

export template <ArithmeticType BaseType, unsigned RowCount, unsigned ColumnCount>
class Matrix
{
    // Helper traits
    static constexpr unsigned TotalComponents = RowCount * ColumnCount;
    static constexpr bool IsMatrixSquare = RowCount == ColumnCount;
    static constexpr bool IsBaseTypeFloat = std::is_floating_point_v<BaseType>;
    static constexpr bool IsBaseTypeIntegral = std::is_integral_v<BaseType>;
    static constexpr BaseType BaseEpsilon = std::numeric_limits<BaseType>::epsilon();

public:

    template <ArithmeticType... ArgTypes>
    explicit constexpr Matrix(const ArgTypes... args)
        requires(TotalComponents >= sizeof...(ArgTypes))
        : values { static_cast<BaseType>(args)... }
    {
    }

    // Copy values flushed to the top left of the matrix
    template <ArithmeticType OtherBaseType, unsigned OtherRowCount, unsigned OtherColumnCount>
    explicit constexpr Matrix(const Matrix<OtherBaseType, OtherRowCount, OtherColumnCount>& other)
        : Matrix {}
    {
        using OtherMatrixType = Matrix<OtherBaseType, OtherRowCount, OtherColumnCount>;

        constexpr unsigned MinRowCount = std::min(RowCount, OtherRowCount);
        constexpr unsigned MinColumnCount = std::min(ColumnCount, OtherColumnCount);

        for (unsigned rowIndex = 0; rowIndex < MinColumnCount; ++rowIndex)
        {
            for (unsigned columnIndex = 0; columnIndex < MinRowCount; ++columnIndex)
            {
                const unsigned index = GetIndex(rowIndex, columnIndex);
                const unsigned otherIndex = OtherMatrixType::GetIndex(rowIndex, columnIndex);

                values[index] = other[otherIndex];
            }
        }
    }

#pragma region BASE

    static constexpr Matrix Identity()
        requires(IsMatrixSquare)
    {
        constexpr unsigned TotalCount = RowCount * ColumnCount;
        constexpr unsigned PaddingCount = RowCount + 1;

        Matrix tmp {};
        for (unsigned i = 0; i < TotalCount; i += PaddingCount)
        {
            tmp[i] = static_cast<BaseType>(1);
        }
        return tmp;
    }

    // Sets all values to be the same
    void Fill(const BaseType fillValue)
    {
        for (BaseType& value : values)
        {
            value = fillValue;
        }
    }

    BaseType* GetMatrix() { return values; }
    BaseType& operator[](const unsigned index) { return values[index]; }
    constexpr BaseType operator[](const unsigned index) const { return values[index]; }

    constexpr Vector<BaseType, RowCount> GetColumn(const unsigned columnIndex) const
    {
        Vector<BaseType, RowCount> column;
        const unsigned offset = columnIndex * RowCount;

        for (unsigned i = 0; i < RowCount; ++i)
        {
            column[i] = values[offset + i];
        }

        return column;
    }
    constexpr Vector<BaseType, ColumnCount> GetRow(const unsigned rowIndex) const
    {
        Vector<BaseType, ColumnCount> row;

        for (unsigned i = 0; i < ColumnCount; ++i)
        {
            row[i] = values[rowIndex + i * RowCount];
        }

        return row;
    }

    template <unsigned ComponentCount>
    void SetColumn(const unsigned columnIndex, const Vector<BaseType, ComponentCount>& column)
        requires(ComponentCount <= RowCount)
    {
        const unsigned offset = columnIndex * RowCount;

        for (unsigned i = 0; i < RowCount; ++i)
        {
            values[offset + i] = column[i];
        }
    }

    template <unsigned ComponentCount>
    void SetRow(const unsigned rowIndex, const Vector<BaseType, ComponentCount>& row)
        requires(ComponentCount <= ColumnCount)
    {
        for (unsigned i = 0; i < ColumnCount; ++i)
        {
            values[rowIndex + i * RowCount] = row[i];
        }
    }

    void SwapRows(const unsigned rowIndexA, const unsigned rowIndexB)
    {
        const Vector<BaseType, ColumnCount>& rowA = GetRow(rowIndexA);
        const Vector<BaseType, ColumnCount>& rowB = GetRow(rowIndexB);
        SetRow(rowIndexB, rowA);
        SetRow(rowIndexA, rowB);
    }

    void SwapColumns(const unsigned columnIndexA, const unsigned columnIndexB)
    {
        const Vector<BaseType, RowCount>& columnA = GetColumn(columnIndexA);
        const Vector<BaseType, RowCount>& columnB = GetColumn(columnIndexB);
        SetColumn(columnIndexB, columnA);
        SetColumn(columnIndexA, columnB);
    }

    static constexpr unsigned GetIndex(const unsigned rowIndex, const unsigned columnIndex) { return columnIndex * RowCount + rowIndex; }

    // Only square matrix can transpose in place
    void Transpose()
        requires(IsMatrixSquare)
    {
        for (unsigned rowIndex = 0; rowIndex < ColumnCount; ++rowIndex)
        {
            for (unsigned columnIndex = rowIndex + 1; columnIndex < RowCount; ++columnIndex)
            {
                std::swap(values[GetIndex(rowIndex, columnIndex)], values[GetIndex(columnIndex, rowIndex)]);
            }
        }
    }

    constexpr Matrix Transposed() const
    {
        Matrix tmp { *this };
        tmp.Transpose();
        return tmp;
    }

    constexpr Matrix<BaseType, ColumnCount, RowCount> Transposed() const
        requires(RowCount != ColumnCount)
    {
        using OtherMatrixType = Matrix<BaseType, ColumnCount, RowCount>;
        OtherMatrixType tmp {};

        for (unsigned rowIndex = 0; rowIndex < ColumnCount; ++rowIndex)
        {
            for (unsigned columnIndex = 0; columnIndex < RowCount; ++columnIndex)
            {
                const unsigned index = GetIndex(rowIndex, columnIndex);
                const unsigned otherIndex = OtherMatrixType::GetIndex(columnIndex, rowIndex);

                tmp[otherIndex] = values[index];
            }
        }

        return tmp;
    }

#pragma endregion BASE

#pragma region MATH

    Matrix& operator+=(const Matrix& other)
    {
        for (unsigned i = 0; i < TotalComponents; ++i)
        {
            values[i] += other.values[i];
        }
        return *this;
    }
    Matrix& operator-=(const Matrix& other)
    {
        for (unsigned i = 0; i < TotalComponents; ++i)
        {
            values[i] -= other.values[i];
        }
        return *this;
    }
    Matrix& operator*=(const BaseType value)
    {
        for (unsigned i = 0; i < TotalComponents; ++i)
        {
            values[i] *= value;
        }
        return *this;
    }
    Matrix& operator/=(const BaseType value)
    {
        for (unsigned i = 0; i < TotalComponents; ++i)
        {
            values[i] /= value;
        }
        return *this;
    }

    constexpr Matrix operator-() const { return Matrix { *this } *= static_cast<BaseType>(-1); }
    constexpr Matrix operator+(const Matrix& other) const { return Matrix { *this } += other; }
    constexpr Matrix operator-(const Matrix& other) const { return Matrix { *this } -= other; }
    constexpr Matrix operator*(const BaseType value) const { return Matrix { *this } *= value; }
    constexpr Matrix operator/(const BaseType value) const { return Matrix { *this } /= value; }

    // Generic matrix multiplication
    template <unsigned OtherRowCount, unsigned OtherColumnCount>
    constexpr Matrix<BaseType, RowCount, OtherColumnCount> operator*(const Matrix<BaseType, OtherRowCount, OtherColumnCount>& other) const
        requires(ColumnCount == OtherRowCount)
    {
        constexpr unsigned ResultComponents = RowCount * OtherColumnCount;

        Matrix<BaseType, RowCount, OtherColumnCount> result {};

        for (unsigned i = 0; i < ResultComponents; ++i)
        {
            const unsigned rowIndex = i % RowCount;
            const unsigned columnIndex = i / OtherColumnCount;

            result[i] = (this->GetRow(rowIndex)).Dot(other.GetColumn(columnIndex));
        }
        return result;
    }

    // Special case where multiplication of matrix with NxN dimensions
    Matrix& operator*=(const Matrix& other)
        requires(ColumnCount == RowCount)
    {
        *this = *this * other;
        return *this;
    }

    constexpr Vector<BaseType, RowCount> operator*(const Vector<BaseType, ColumnCount>& vector)
    {
        Vector<BaseType, RowCount> tmp;
        for (unsigned i = 0; i < RowCount; ++i)
        {
            tmp[i] = GetRow(i).Dot(vector);
        }
        return tmp;
    }

    constexpr BaseType Determinant() const
        requires(IsMatrixSquare && IsBaseTypeFloat)
    {
        constexpr unsigned Dimension = RowCount;

        // Compute row echelon form
        Matrix tmp { *this };

        for (unsigned i = 0; i < Dimension; ++i)
        {
            for (unsigned j = i + 1; j < Dimension; ++j)
            {
                const BaseType factor = tmp.values[GetIndex(i, j)] / tmp.values[GetIndex(i, i)];
                for (unsigned k = i; k < Dimension; ++k)
                {
                    tmp.values[GetIndex(j, k)] -= factor * tmp.values[GetIndex(i, k)];
                }
            }
        }

        // Compute determinant by multipliation of diagonals
        BaseType determinant = static_cast<BaseType>(1);
        for (unsigned i = 0; i < Dimension; ++i)
        {
            determinant *= tmp.values[GetIndex(i, i)];
        }

        return determinant;
    }

    constexpr Matrix Inverted() const
        requires(IsMatrixSquare && IsBaseTypeFloat)
    {
        Matrix tmp { *this };
        tmp.Invert();
        return tmp;
    }

    template <unsigned Components>
    static constexpr Matrix Scale(const Vector<BaseType, Components>& vector)
        requires(IsMatrixSquare)
    {
        constexpr unsigned MinCount = std::min(Components, RowCount);

        Matrix tmp {};
        for (unsigned i = 0; i < MinCount; ++i)
        {
            tmp.values[GetIndex(i, i)] = vector[i];
        }
        return tmp;
    }

    template <ArithmeticType... ArgTypes>
    static constexpr Matrix Scale(const ArgTypes... args)
        requires(IsMatrixSquare && sizeof...(ArgTypes) <= RowCount)
    {
        return Matrix::Scale(Vector { static_cast<BaseType>(args)... });
    }

#pragma endregion MATH

#pragma region MATRIX2x2

    void Invert()
        requires(RowCount == 2 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType determinant = Determinant();

        if (std::abs(determinant) <= BaseEpsilon)
        {
            throw std::logic_error("Trying to invert a matrix with determinant close to 0");
            return;
        } 

        std::swap(values[0].values[3]);
        values[1] = -values[1];
        values[2] = -values[2];

        *this /= determinant;
    }

    static constexpr Matrix Rotate(BaseType angle)
        requires(RowCount == 2 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { std::cos(angle), std::sin(angle), -std::sin(angle), std::cos(angle) };
    }

#pragma endregion MATRIX2x2

#pragma region MATRIX3x3

    void Invert()
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType determinant = Determinant();

        if (std::abs(determinant) <= BaseEpsilon)
        {
            throw std::logic_error("Trying to invert a matrix with determinant close to 0");
            return;
        }

        Matrix tmp {
            values[4] * values[8] - values[5] * values[7],
            values[7] * values[2] - values[8] * values[1],
            values[1] * values[5] - values[2] * values[4],
            values[5] * values[6] - values[3] * values[8],
            values[0] * values[8] - values[2] * values[6],
            values[2] * values[3] - values[0] * values[5],
            values[3] * values[7] - values[4] * values[6],
            values[6] * values[1] - values[7] * values[0],
            values[0] * values[4] - values[1] * values[3],
        };

        *this = tmp / determinant;
    }

    static constexpr Matrix RotateX(const BaseType angle)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType c = std::cos(angle);
        const BaseType s = std::sin(angle);

        Matrix tmp {};

        tmp[4] = c;
        tmp[5] = s;
        tmp[7] = -s;
        tmp[8] = c;

        return tmp;
    }
    static constexpr Matrix RotateY(const BaseType angle)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType c = std::cos(angle);
        const BaseType s = std::sin(angle);

        Matrix tmp {};

        tmp[0] = c;
        tmp[2] = -s;
        tmp[6] = s;
        tmp[8] = c;

        return tmp;
    }
    static constexpr Matrix RotateZ(const BaseType angle)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType c = std::cos(angle);
        const BaseType s = std::sin(angle);

        Matrix tmp {};

        tmp[0] = c;
        tmp[1] = s;
        tmp[3] = -s;
        tmp[4] = c;

        return tmp;
    }
    static constexpr Matrix Rotate(const Vector<BaseType, RowCount>& rotateAngles)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return RotateX(rotateAngles[0]) * RotateY(rotateAngles[1]) * RotateZ(rotateAngles[2]);
    }
    static constexpr Matrix Rotate(const BaseType pitch, const BaseType roll, const BaseType yaw)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix::Rotate(Vector<BaseType, RowCount> { pitch, roll, yaw });
    }

    static constexpr Matrix Rotate(const Vector<BaseType, RowCount>& axis, const BaseType angle)
        requires(RowCount == 3 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType c = std::cos(angle);                // cosine
        const BaseType s = std::sin(angle);                // sine
        const BaseType c1 = static_cast<BaseType>(1) - c;  // 1 - c

        // build rotation matrix
        return Matrix {
            axis[0] * axis[0] * c1 + c,
            axis[0] * axis[1] * c1 + axis[2] * s,
            axis[0] * axis[2] * c1 - axis[1] * s,

            axis[0] * axis[1] * c1 - axis[2] * s,
            axis[1] * axis[1] * c1 + c,
            axis[1] * axis[2] * c1 + axis[0] * s,

            axis[0] * axis[2] * c1 + axis[1] * s,
            axis[1] * axis[2] * c1 - axis[0] * s,
            axis[2] * axis[2] * c1 + c,
        };
    }
#pragma endregion MATRIX3x3

#pragma region MATRIX4x4

    constexpr BaseType GetCoFactor(BaseType m0, BaseType m1, BaseType m2, BaseType m3, BaseType m4, BaseType m5, BaseType m6, BaseType m7, BaseType m8) const
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return m0 * (m4 * m8 - m5 * m7) - m1 * (m3 * m8 - m5 * m6) + m2 * (m3 * m7 - m4 * m6);
    }

    void Invert()
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType determinant = Determinant();

        if (std::abs(determinant) <= BaseEpsilon)
        {
            throw std::logic_error("Trying to invert a matrix with determinant close to 0");
            return;
        }

        // get cofactors of minor matrices
        BaseType cofactor0 = GetCoFactor(values[5], values[6], values[7], values[9], values[10], values[11], values[13], values[14], values[15]);
        BaseType cofactor1 = GetCoFactor(values[4], values[6], values[7], values[8], values[10], values[11], values[12], values[14], values[15]);
        BaseType cofactor2 = GetCoFactor(values[4], values[5], values[7], values[8], values[9], values[11], values[12], values[13], values[15]);
        BaseType cofactor3 = GetCoFactor(values[4], values[5], values[6], values[8], values[9], values[10], values[12], values[13], values[14]);

        // get rest of cofactors for adj(M)
        BaseType cofactor4 = GetCoFactor(values[1], values[2], values[3], values[9], values[10], values[11], values[13], values[14], values[15]);
        BaseType cofactor5 = GetCoFactor(values[0], values[2], values[3], values[8], values[10], values[11], values[12], values[14], values[15]);
        BaseType cofactor6 = GetCoFactor(values[0], values[1], values[3], values[8], values[9], values[11], values[12], values[13], values[15]);
        BaseType cofactor7 = GetCoFactor(values[0], values[1], values[2], values[8], values[9], values[10], values[12], values[13], values[14]);

        BaseType cofactor8 = GetCoFactor(values[1], values[2], values[3], values[5], values[6], values[7], values[13], values[14], values[15]);
        BaseType cofactor9 = GetCoFactor(values[0], values[2], values[3], values[4], values[6], values[7], values[12], values[14], values[15]);
        BaseType cofactor10 = GetCoFactor(values[0], values[1], values[3], values[4], values[5], values[7], values[12], values[13], values[15]);
        BaseType cofactor11 = GetCoFactor(values[0], values[1], values[2], values[4], values[5], values[6], values[12], values[13], values[14]);

        BaseType cofactor12 = GetCoFactor(values[1], values[2], values[3], values[5], values[6], values[7], values[9], values[10], values[11]);
        BaseType cofactor13 = GetCoFactor(values[0], values[2], values[3], values[4], values[6], values[7], values[8], values[10], values[11]);
        BaseType cofactor14 = GetCoFactor(values[0], values[1], values[3], values[4], values[5], values[7], values[8], values[9], values[11]);
        BaseType cofactor15 = GetCoFactor(values[0], values[1], values[2], values[4], values[5], values[6], values[8], values[9], values[10]);

        // build inverse matrix = adj(M) / det(M)
        // adjugate of M is the transpose of the cofactor matrix of M
        const BaseType invDeterminant = 1.0f / determinant;
        values[0] = invDeterminant * cofactor0;
        values[1] = -invDeterminant * cofactor4;
        values[2] = invDeterminant * cofactor8;
        values[3] = -invDeterminant * cofactor12;
        values[4] = -invDeterminant * cofactor1;
        values[5] = invDeterminant * cofactor5;
        values[6] = -invDeterminant * cofactor9;
        values[7] = invDeterminant * cofactor13;
        values[8] = invDeterminant * cofactor2;
        values[9] = -invDeterminant * cofactor6;
        values[10] = invDeterminant * cofactor10;
        values[11] = -invDeterminant * cofactor14;
        values[12] = -invDeterminant * cofactor3;
        values[13] = invDeterminant * cofactor7;
        values[14] = -invDeterminant * cofactor11;
        values[15] = invDeterminant * cofactor15;
    }

    void Decompose(Vector<BaseType, 3>& translate, Vector<BaseType, 3>& rotation, Vector<BaseType, 3>& scale) const
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        using ReturnVectorType = Vector<BaseType, 3>;

        Matrix tmp { *this };

        const ReturnVectorType column0 { GetColumn(0) };
        const ReturnVectorType column1 { GetColumn(1) };
        const ReturnVectorType column2 { GetColumn(2) };

        scale[0] = column0.Length();
        scale[1] = column1.Length();
        scale[2] = column2.Length();

        tmp.SetColumn(0, column0.Normalized());
        tmp.SetColumn(1, column1.Normalized());
        tmp.SetColumn(2, column2.Normalized());

        rotation[0] = std::atan2(tmp[6], tmp[10]);
        rotation[1] = std::atan2(-tmp[2], std::sqrt(tmp[6] * tmp[6] + tmp[10] * tmp[10]));
        rotation[2] = std::atan2(tmp[1], tmp[0]);

        translate[0] = tmp[12];
        translate[1] = tmp[13];
        translate[2] = tmp[14];
    }

    static constexpr Matrix Translate(const Vector<BaseType, 3>& translate)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        Matrix tmp {};
        tmp.SetColumn(3, translate);
        return tmp;
    }

    static constexpr Matrix Rotate(const Vector<BaseType, 3>& axis, const BaseType angle)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { Matrix<BaseType, 3, 3>::Rotate(axis, angle) };
    }

    static constexpr Matrix RotateX(const BaseType angle)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { Matrix<BaseType, 3, 3>::RotateX(angle) };
    }
    static constexpr Matrix RotateY(const BaseType angle)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { Matrix<BaseType, 3, 3>::RotateY(angle) };
    }
    static constexpr Matrix RotateZ(const BaseType angle)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { Matrix<BaseType, 3, 3>::RotateZ(angle) };
    }
    static constexpr Matrix FrustumPerspective(const BaseType left, const BaseType right, const BaseType bottom, const BaseType top, const BaseType near, const BaseType far)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix { 2 * near / (right - left), 0, 0, 0, 0, 2 * near / (top - bottom), 0, 0, (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1, 0,
            0, -(2 * far * near) / (far - near), 0 };
    }

    static constexpr Matrix FrustumOrthographic(const BaseType left, const BaseType right, const BaseType bottom, const BaseType top, const BaseType near, const BaseType far)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        return Matrix {
            2 / (right - left),
            0,
            0,
            0,
            0,
            2 / (top - bottom),
            0,
            0,
            0,
            0,
            -2 / (far - near),
            0,
            -(right + left) / (right - left),
            -(top + bottom) / (top - bottom),
            -(far + near) / (far - near),
            0,
        };
    }

    static constexpr Matrix Perspective(const BaseType fov, const BaseType aspectRatio, const BaseType near, const BaseType far)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        const BaseType tanFov = std::tan(fov / 2);
        const BaseType height = near * tanFov;
        const BaseType width = height * aspectRatio;

        return Matrix::FrustumOrthographic(-width, width, -height, height, near, far);
    }

    static constexpr Matrix LookAt(const Vector<BaseType, 3>& eye, const Vector<BaseType, 3>& center, const Vector<BaseType, 3>& worldUp)
        requires(RowCount == 4 && IsMatrixSquare && IsBaseTypeFloat)
    {
        Vector<BaseType, 3> forward = eye - center;
        forward.Normalize();
        Vector<BaseType, 3> right = worldUp.Cross(forward);
        right.Normalize();
        Vector<BaseType, 3> up = forward.Cross(right);
        up.Normalize();

        Matrix transform;
        transform.SetRow(0, right);
        transform.SetRow(1, up);
        transform.SetRow(2, forward);
        transform *= Matrix::Translate(-eye);
        return transform;
    }

#pragma endregion MATRIX4x4

    friend std::ostream& operator<<(std::ostream& os, const Matrix& other);

private:

    // Data is stored in column major
    BaseType values[RowCount * ColumnCount];
};

export template <typename BaseType, unsigned RowCount, unsigned ColumnCount>
std::ostream& operator<<(std::ostream& os, const Matrix<BaseType, RowCount, ColumnCount>& rhs)
{
    // os << "Matrix4[";
    // os << rhs[0] << ", " << rhs[4] << ", " << rhs[8] << ", " << rhs[12] << std::endl;
    // os << rhs[1] << ", " << rhs[5] << ", " << rhs[9] << ", " << rhs[13] << std::endl;
    // os << rhs[2] << ", " << rhs[6] << ", " << rhs[10] << ", " << rhs[14] << std::endl;
    // os << rhs[3] << ", " << rhs[7] << ", " << rhs[11] << ", " << rhs[15] << "]" << std::endl;
    return os;
}
