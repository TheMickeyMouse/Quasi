#pragma once
#include <ostream>

#include "Vector.h"

namespace Maths
{
    class Matrix3D
    {
    private:
        float m11, m12, m13, m14;
        float m21, m22, m23, m24;
        float m31, m32, m33, m34;
        float m41, m42, m43, m44;
    public:
        Matrix3D() : m11(1.0f), m12(0.0f), m13(0.0f), m14(0.0f),
                     m21(0.0f), m22(1.0f), m23(0.0f), m24(0.0f),
                     m31(0.0f), m32(0.0f), m33(1.0f), m34(0.0f),
                     m41(0.0f), m42(0.0f), m43(0.0f), m44(1.0f) {}
        Matrix3D(float vals[16], bool horizontalPacking = true);
        Matrix3D(float m11, float m12, float m13, float m14,
                 float m21, float m22, float m23, float m24,
                 float m31, float m32, float m33, float m34,
                 float m41, float m42, float m43, float m44,
                 bool horizontalPacking = true);
        Matrix3D(Vector4 arrs[4], bool verticalPacking = true);
        Matrix3D(const Vector4& r1, const Vector4& r2, const Vector4& r3, const Vector4& r4, bool verticalPacking = true);

        const float* GetInRow() const;
        const Vector4* GetRows() const;

        void Translate(const Vector3& translation);
        static Matrix3D TranslateMat(const Vector3& translation, const Matrix3D& mat = {});
        void Scale(const Vector3& scale);
        static Matrix3D ScaleMat(const Vector3& scale, const Matrix3D& mat = {});
        void Rotate(const Vector3& rotation);
        static Matrix3D RotateMat(const Vector3& rotation, const Matrix3D& mat = {});

        static Matrix3D OrthoProjection(float left, float right, float down, float up, float near, float front);
        static Matrix3D PerspectiveProjection(float left, float right, float down, float up, float near, float far);
        static Matrix3D PerspectiveProjectionFOV(float fovDeg, float aspect, float near, float far);
        static Matrix3D Transform(const Vector3& translate, const Vector3& scale, const Vector3& rotate);

        [[nodiscard]] Matrix3D Transpose() const;
        [[nodiscard]] float Determinate() const;
        [[nodiscard]] Matrix3D Adjugate() const;
        [[nodiscard]] Matrix3D Inverse() const;

        static Matrix3D UnitAxis(Direction3D x, Direction3D y, Direction3D z);

        float operator[](unsigned i) { return *((float*)this + i); }

        Vector3 operator* (const Vector3& v) const;
        Vector4 operator* (const Vector4& v) const;
        Matrix3D operator* (const Matrix3D& m) const;
        Matrix3D operator* (float x) const;
        
        friend std::ostream& operator<<(std::ostream& stream, const Matrix3D& mat);
    };

    std::ostream& operator<<(std::ostream& stream, const Matrix3D& mat);
}