#include "Matrix.h"

#include <complex>
#include <iomanip>

namespace Maths {
    Matrix3D::Matrix3D(float vals[16], bool horizontalPacking) {
        if (horizontalPacking) {
            m11 = vals[0 +0]; m12 = vals[0 +1]; m13 = vals[0 +2]; m14 = vals[0 +3];
            m21 = vals[4 +0]; m22 = vals[4 +1]; m23 = vals[4 +2]; m24 = vals[4 +3];
            m31 = vals[8 +0]; m32 = vals[8 +1]; m33 = vals[8 +2]; m34 = vals[8 +3];
            m41 = vals[12+0]; m42 = vals[12+1]; m43 = vals[12+2]; m44 = vals[12+3];
        } else {
            m11 = vals[0 +0]; m21 = vals[0 +1]; m31 = vals[0 +2]; m41 = vals[0 +3];
            m12 = vals[4 +4]; m22 = vals[4 +5]; m32 = vals[4 +6]; m42 = vals[4 +7];
            m13 = vals[8 +4]; m23 = vals[8 +5]; m33 = vals[8 +6]; m43 = vals[8 +7];
            m14 = vals[12+4]; m24 = vals[12+5]; m34 = vals[12+6]; m44 = vals[12+7];
        }
    }

    Matrix3D::Matrix3D(float _m11, float _m12, float _m13, float _m14,
                       float _m21, float _m22, float _m23, float _m24,
                       float _m31, float _m32, float _m33, float _m34,
                       float _m41, float _m42, float _m43, float _m44, bool horizontalPacking) {
        if (horizontalPacking) {
            m11 = _m11; m12 = _m12; m13 = _m13; m14 = _m14;
            m21 = _m21; m22 = _m22; m23 = _m23; m24 = _m24;
            m31 = _m31; m32 = _m32; m33 = _m33; m34 = _m34;
            m41 = _m41; m42 = _m42; m43 = _m43; m44 = _m44;
        } else {
            m11 = _m11; m21 = _m21; m31 = _m31; m41 = _m41;
            m12 = _m12; m22 = _m22; m32 = _m32; m42 = _m42;
            m13 = _m13; m23 = _m23; m33 = _m33; m43 = _m43;
            m14 = _m14; m24 = _m24; m34 = _m34; m44 = _m44;
        }
    }

    Matrix3D::Matrix3D(Vector4 arrs[4], bool verticalPacking) {
        if (!verticalPacking) {
            m11 = arrs[0].x; m12 = arrs[0].y; m13 = arrs[0].z; m14 = arrs[0].w;
            m21 = arrs[1].x; m22 = arrs[1].y; m23 = arrs[1].z; m24 = arrs[1].w;
            m31 = arrs[2].x; m32 = arrs[2].y; m33 = arrs[2].z; m34 = arrs[2].w;
            m41 = arrs[3].x; m42 = arrs[3].y; m43 = arrs[3].z; m44 = arrs[3].w;
        } else {
            m11 = arrs[0].x; m21 = arrs[0].y; m31 = arrs[0].z; m41 = arrs[0].w;
            m12 = arrs[1].x; m22 = arrs[1].y; m32 = arrs[1].z; m42 = arrs[1].w;
            m13 = arrs[2].x; m23 = arrs[2].y; m33 = arrs[2].z; m43 = arrs[2].w;
            m14 = arrs[3].x; m24 = arrs[3].y; m34 = arrs[3].z; m44 = arrs[3].w;
        }
    }

    Matrix3D::Matrix3D(const Vector4& r1, const Vector4& r2, const Vector4& r3, const Vector4& r4, bool verticalPacking) {
        if (!verticalPacking) {
            m11 = r1.x; m12 = r1.y; m13 = r1.z; m14 = r1.w;
            m21 = r2.x; m22 = r2.y; m23 = r2.z; m24 = r2.w;
            m31 = r3.x; m32 = r3.y; m33 = r3.z; m34 = r3.w;
            m41 = r4.x; m42 = r4.y; m43 = r4.z; m44 = r4.w;
        } else {
            m11 = r1.x; m21 = r1.y; m31 = r1.z; m41 = r1.w;
            m12 = r2.x; m22 = r2.y; m32 = r2.z; m42 = r2.w;
            m13 = r3.x; m23 = r3.y; m33 = r3.z; m43 = r3.w;
            m14 = r4.x; m24 = r4.y; m34 = r4.z; m44 = r4.w;
        }
    }

    const float* Matrix3D::GetInRow() const {
        return (const float*)this;
    }

    const Vector4* Matrix3D::GetRows() const {
        return (const Vector4*)this;
    }

    void Matrix3D::Translate(const Vector3& translation) {
        //{{1, 0, 0, t x}, {0, 1, 0, t y}, {0, 0, 1, t z}, {0, 0, 0, 1}} . {{a, b, c, d}, {e, f, g, h}, {I, j, k, l}, {0, 0, 0, 1}}
        m14 += translation.x;
        m24 += translation.y;
        m34 += translation.z;
    }

    Matrix3D Matrix3D::TranslateMat(const Vector3& translation, const Matrix3D& mat) {
        return { mat.m11, mat.m12, mat.m13, mat.m14 + translation.x,
                 mat.m21, mat.m22, mat.m23, mat.m24 + translation.y,
                 mat.m31, mat.m32, mat.m33, mat.m34 + translation.z,
                 mat.m41, mat.m42, mat.m43, mat.m44                  };
    }

    void Matrix3D::Scale(const Vector3& scale) {
        m11 *= scale.x; m12 *= scale.x; m13 *= scale.x; m14 *= scale.x;
        m21 *= scale.y; m22 *= scale.y; m23 *= scale.y; m24 *= scale.y;
        m31 *= scale.z; m32 *= scale.z; m33 *= scale.z; m34 *= scale.z;
    }

    Matrix3D Matrix3D::ScaleMat(const Vector3& scale, const Matrix3D& mat) {
        return { mat.m11 * scale.x, mat.m12 * scale.x, mat.m13 * scale.x, mat.m14 * scale.x,
                 mat.m21 * scale.y, mat.m22 * scale.y, mat.m23 * scale.y, mat.m24 * scale.y,
                 mat.m31 * scale.z, mat.m32 * scale.z, mat.m33 * scale.z, mat.m34 * scale.z,
                 mat.m41,           mat.m42,           mat.m43,           mat.m44            };
    }

    void Matrix3D::Rotate(const Vector3& rotation) {
        // https://en.wikipedia.org/wiki/Rotation_matrix
        const float cx = cos(rotation.x);
        const float sx = sin(rotation.x);
        const float cy = cos(rotation.y);
        const float sy = sin(rotation.y);
        const float cz = cos(rotation.z);
        const float sz = sin(rotation.z);
        Matrix3D rotm = { cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz, 0,
                          cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz, 0,
                         -sy     , sx * cy               , cx * cy               , 0,
                          0      , 0                     , 0                     , 1 };
        *this = rotm * *this;
    }

    Matrix3D Matrix3D::RotateMat(const Vector3& rotation, const Matrix3D& mat) {
        const float cx = cos(rotation.x);
        const float sx = sin(rotation.x);
        const float cy = cos(rotation.y);
        const float sy = sin(rotation.y);
        const float cz = cos(rotation.z);
        const float sz = sin(rotation.z);
        Matrix3D rotm = { cy * cz, sx * sy * cz - cx * sz, cx * sy * cz + sx * sz, 0,
                          cy * sz, sx * sy * sz + cx * cz, cx * sy * sz - sx * cz, 0,
                         -sy     , sx * cy               , cx * cy               , 0,
                          0      , 0                     , 0                     , 1};
        return rotm * mat;
    }

    Matrix3D Matrix3D::OrthoProjection(float l, float r, float d, float u, float n, float f) {
        const float dx = r - l; 
        const float dy = u - d; 
        const float dz = f - n; 
        return {
            2.0f / dx, 0.0f,      0.0f,      -(r + l) / dx,
            0.0f,      2.0f / dy, 0.0f,      -(u + d) / dy,
            0.0f,      0.0f,     -2.0f / dz, -(f + n) / dz,
            0.0f,      0.0f,      0.0f,       1.0f
        };
    }

    Matrix3D Matrix3D::PerspectiveProjection(float l, float r, float d, float u, float n, float f) {
        // heres the math: http://www.songho.ca/opengl/gl_projectionmatrix.html
        const float dx = r - l;
        const float dy = u - d;
        const float dz = f - n;
        return {
            2 * n / dx, 0.0f,       (r + l) / dx,  0.0f,
            0.0f,       2 * n / dy, (u + d) / dy,  0.0f,
            0.0f,       0.0f,      -(n + f) / dz, -2 * n * f / dz,
            0.0f,       0.0f,      -1.0f,          0.0f
        };
    }

    Matrix3D Matrix3D::PerspectiveProjectionFOV(float fovDeg, float aspect, float near, float far) {
        constexpr float PI = 3.14159265358979323846f;
        float fovRad = fovDeg * PI / 180.0f;
        float y = 1.0f / std::tan(fovRad / 2);
        float x = y / aspect;
        float dz = far - near;
        return {
            x,    0.0f,  0.0f,               0.0f,
            0.0f, y,     0.0f,               0.0f,
            0.0f, 0.0f, -(far + near) / dz, -2 * far * near / dz,
            0.0f, 0.0f, -1.0f,               0.0f
        };
    }

    Matrix3D Matrix3D::Transform(const Vector3& translate, const Vector3& scale, const Vector3& rotate) {
        return TranslateMat(translate) * RotateMat(rotate) * ScaleMat(scale);
    }

    Matrix3D Matrix3D::Transpose() const {
#define MROW4(Y) m1##Y, m2##Y, m3##Y, m4##Y
        return { MROW4(1), MROW4(2), MROW4(3), MROW4(4) };
#undef MROW4
    }

#define EXPAND(X) X
#define DET3_(E11, E12, E13, E21, E22, E23, E31, E32, E33) E11 * E22 * E33 + E12 * E23 * E31 + E13 * E21 * E32 - \
                                                           E13 * E22 * E31 - E12 * E21 * E33 - E11 * E23 * E32
#define DET3(...) EXPAND(DET3_(__VA_ARGS__))
#define DETROW3(R1, R2, R3) (DET3(m##R1##2, m##R1##3, m##R1##4, m##R2##2, m##R2##3, m##R2##4, m##R3##2, m##R3##3, m##R3##4))

    // https://semath.info/src/inverse-cofactor-ex4.html for matrix math
    float Matrix3D::Determinate() const {
        return m11 * DETROW3(2, 3, 4) - m21 * DETROW3(1, 3, 4) + m31 * DETROW3(1, 2, 4) - m41 * DETROW3(1, 2, 3);
    }

// doing this because c++, i guess
#define CAT_LIT(A, B) A ## B
#define CAT(A, B) CAT_LIT(A, B)
#define IGNORE(I) CAT(IGNORE_, I)
#define IGNORE_1 2, 3, 4
#define IGNORE_2 1, 3, 4
#define IGNORE_3 1, 2, 4
#define IGNORE_4 1, 2, 3
#define IGNORE_ELEM(I, J) IGNORE(I), IGNORE(J)
#define SUBMAT_(I1, I2, I3, J1, J2, J3) CAT(m, CAT(I1, J1)), CAT(m, CAT(I1, J2)), CAT(m, CAT(I1, J3)), CAT(m, CAT(I2, J1)), CAT(m, CAT(I2, J2)), CAT(m, CAT(I2, J3)), CAT(m, CAT(I3, J1)), CAT(m, CAT(I3, J2)), CAT(m, CAT(I3, J3))
#define SUBMAT(...) EXPAND(SUBMAT_(__VA_ARGS__))
#define ADJUGATE_ELEM(I, J) (DET3(SUBMAT(IGNORE_ELEM(J, I))))
    
    Matrix3D Matrix3D::Adjugate() const {
        return {  ADJUGATE_ELEM(1, 1), -ADJUGATE_ELEM(1, 2),  ADJUGATE_ELEM(1, 3), -ADJUGATE_ELEM(1, 4),
                 -ADJUGATE_ELEM(2, 1),  ADJUGATE_ELEM(2, 2), -ADJUGATE_ELEM(2, 3),  ADJUGATE_ELEM(2, 4),
                  ADJUGATE_ELEM(3, 1), -ADJUGATE_ELEM(3, 2),  ADJUGATE_ELEM(3, 3), -ADJUGATE_ELEM(3, 4),
                 -ADJUGATE_ELEM(4, 1),  ADJUGATE_ELEM(4, 2), -ADJUGATE_ELEM(4, 3),  ADJUGATE_ELEM(4, 4) };
    }

    Matrix3D Matrix3D::Inverse() const {
        float det = Determinate();
        return Adjugate() * (1 / det);
    }

#undef CAT_LIT
#undef CAT
#undef IGNORE
#undef IGNORE_1
#undef IGNORE_2
#undef IGNORE_3
#undef IGNORE_4
#undef IGNORE_ELEM
#undef SUBMAT_
#undef SUBMAT
#undef ADJUGATE_ELEM

#undef DET3_
#undef DET3
#undef DETROW3

    Matrix3D Matrix3D::UnitAxis(Direction3D x, Direction3D y, Direction3D z) {
        return { Vector4 { x, 0 }, Vector4 { y, 0 }, Vector4 { z, 0 }, { 0, 0, 0, 1 } };
    }

    Vector3 Matrix3D::operator*(const Vector3& v) const {
        return { m11 * v.x + m12 * v.y + m13 * v.z + m14,
                 m21 * v.x + m22 * v.y + m23 * v.z + m24,
                 m31 * v.x + m32 * v.y + m33 * v.z + m34  };
    }

    Vector4 Matrix3D::operator*(const Vector4& v) const {
        return { m11 * v.x + m12 * v.y + m13 * v.z + m14 * v.w,
                 m21 * v.x + m22 * v.y + m23 * v.z + m24 * v.w,
                 m31 * v.x + m32 * v.y + m33 * v.z + m34 * v.w,
                 m41 * v.x + m42 * v.y + m43 * v.z + m44 * v.w  };
    }

    Matrix3D Matrix3D::operator*(const Matrix3D& m) const {
#define MATMUL4(X, Y) m##Y##1 * m.m1##X + m##Y##2 * m.m2##X + m##Y##3 * m.m3##X + m##Y##4 * m.m4##X
#define MATMUL4R(Y) MATMUL4(1, Y), MATMUL4(2, Y), MATMUL4(3, Y), MATMUL4(4, Y)
        
        return { MATMUL4R(1), MATMUL4R(2), MATMUL4R(3), MATMUL4R(4) };

#undef MATMUL4
#undef MATMUL4R
    }

    Matrix3D Matrix3D::operator*(float x) const {
        return { m11 * x, m12 * x, m13 * x, m14 * x,
                 m21 * x, m22 * x, m23 * x, m24 * x,
                 m31 * x, m32 * x, m33 * x, m34 * x,
                 m41 * x, m42 * x, m43 * x, m44 * x };
    }

    std::ostream& operator<<(std::ostream& stream, const Matrix3D& mat) {
        stream << std::fixed << std::setprecision(3);
        stream << '[' << mat.m11 << ',' << mat.m12 << ',' << mat.m13 << ',' << mat.m14 << ']' << '\n';
        stream << '[' << mat.m21 << ',' << mat.m22 << ',' << mat.m23 << ',' << mat.m24 << ']' << '\n';
        stream << '[' << mat.m31 << ',' << mat.m32 << ',' << mat.m33 << ',' << mat.m34 << ']' << '\n';
        stream << '[' << mat.m41 << ',' << mat.m42 << ',' << mat.m43 << ',' << mat.m44 << ']';// << '\n'
        stream << std::setprecision(-1);
        return stream;
    }
}
