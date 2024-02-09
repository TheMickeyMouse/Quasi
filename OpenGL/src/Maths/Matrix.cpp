#include "Matrix.h"

#include <complex>
#include <iomanip>

#include "Constants.h"

namespace Maths {
    const float* mat4x4::get_in_col() const {
        return (const float*)this;
    }

    const fvec4* mat4x4::get_cols() const {
        return (const fvec4*)this;
    }

    void mat4x4::translate(const fvec3& translation) {
        w += translation.with_w(0);
    }

    mat4x4 mat4x4::translate_mat(const fvec3& translation, const mat4x4& mat) {
        return { mat.x, mat.y, mat.z, mat.w + translation.with_w(0) };
    }

    void mat4x4::scale(const fvec3& scale) {
        x *= scale.x;
        y *= scale.y;
        z *= scale.z;
    }

    mat4x4 mat4x4::scale_mat(const fvec3& scale, const mat3D& mat) {
        return { mat.x * scale.x, mat.y * scale.y, mat.z * scale.z, mat.w };
    }

    void mat4x4::rotate(const fvec3& rotation) {
        *this = rotate_identity(rotation) * *this;
    }

    mat4x4 mat4x4::rotate_mat(const fvec3& rotation, const mat3D& mat) {
        return rotate_identity(rotation) * mat;
    }

    mat4x4 mat4x4::rotate_identity(const fvec3& rotation) {
        // https://en.wikipedia.org/wiki/Rotation_matrix
        // not most efficient but i hate math so...
        return rotate_y(rotation.y) * rotate_x(rotation.x) * rotate_z(rotation.z);
    }

    mat4x4 mat4x4::rotate_x(float roll) {
        const float sx = sinf(roll);
        const float cx = cosf(roll);
        return { { 1, 0,   0,  0 },
                 { 0, cx, -sx, 0 },
                 { 0, sx,  cx, 0 } };
    }

    mat4x4 mat4x4::rotate_y(float pitch) {
        const float sy = sinf(pitch);
        const float cy = cosf(pitch);
        return { { cy, 0, -sy, 0 },
                 { 0,  1,  0,  0 },
                 { sy, 0,  cy, 0 } };
    }

    mat4x4 mat4x4::rotate_z(float yaw) {
        const float sz = sinf(yaw);
        const float cz = cosf(yaw);
        return { {  cz, sz, 0, 0 },
                 { -sz, cz, 0, 0 },
                 {  0,   0, 1, 0 } };
    }

    mat4x4 mat4x4::ortho_projection(const rect3f& box) {
        const float dx = box.width(),  sx = box.min.x + box.max.x,
                    dy = box.height(), sy = box.min.y + box.max.y,
                    dz = box.depth(),  sz = box.min.z + box.max.z;
        return { { +2  / dx,  0,        0,       0 },
                 { 0,        +2  / dy,  0,       0 },
                 { 0,         0,       -2  / dz, 0 },
                 { -sx / dx, -sy / dy, -sz / dz, 1 } };
    }

    mat4x4 mat4x4::perspective_projection(const rect3f& box) {
        // heres the math: http://www.songho.ca/opengl/gl_projectionmatrix.html
        const float n = box.min.z, f = box.max.z,
                    dx = box.width(),  sx = box.min.x + box.max.x,
                    dy = box.height(), sy = box.min.y + box.max.y,
                    dz = box.depth(),  sz = box.min.z + box.max.z;
        return { { 2 * n / dx, 0,          0,               0 },
                 { 0,          2 * n / dy, 0,               0 },
                 { sx    / dx, sy    / dx, -sz / dx,       -1 },
                 { 0,          0,          -2 * n * f / dz, 0 } };
    }

    mat4x4 mat4x4::perspective_fov(float fovDeg, float aspect, float near, float far) {
        float fovRad = fovDeg * PI / 180.0f;
        float y = 1.0f / std::tan(fovRad / 2);
        float x = y / aspect;
        float dz = far - near;
        float zsum  = (far + near) / dz;
        float zprod = (far * near) / dz;
        return { { x, 0,  0,         0 },
                 { 0, y,  0,         0 },
                 { 0, 0, -zsum,     -1 },
                 { 0, 0, -2 * zprod, 0 }
        };
    }

    mat4x4 mat4x4::transform(const fvec3& translate, const fvec3& scale, const fvec3& rotate) {
        return translate_mat(translate) * rotate_identity(rotate) * scale_mat(scale);
    }

    // * warning! LOTS OF MACROS AHEAD!
#define MROW4(U) { x.U, y.U, z.U, w.U }
    mat4x4 mat4x4::transpose() const {
        return { MROW4(x), MROW4(y), MROW4(z), MROW4(w) };
    }
#undef MROW4

#define DET3_(E11, E12, E13, E21, E22, E23, E31, E32, E33) E11 * E22 * E33 + E12 * E23 * E31 + E13 * E21 * E32 - /* NOLINT(bugprone-macro-parentheses) */ \
                                                           E13 * E22 * E31 - E12 * E21 * E33 - E11 * E23 * E32   /* NOLINT(bugprone-macro-parentheses) */
#define DET3(...) STDU_UNARY(DET3_(__VA_ARGS__))
#define DETMAT3(R1, R2, R3) (DET3(y.R1, z.R1, w.R1, y.R2, z.R2, w.R2, y.R3, z.R3, w.R3)) /* NOLINT(bugprone-macro-parentheses) */

    // https://semath.info/src/inverse-cofactor-ex4.html for matrix math
    float mat4x4::det() const {
        return x.x * DETMAT3(y, z, w) - x.y * DETMAT3(x, z, w) + x.z * DETMAT3(x, y, w) - x.w * DETMAT3(x, y, z);
    }

    // doing this because c++, i guess
    // * haha macro hacks go brrrrr
#define IGNORE(I) STDU_CAT(IGNORE_, I)
#define IGNORE_x y, z, w
#define IGNORE_y x, z, w
#define IGNORE_z x, y, w
#define IGNORE_w x, y, z
#define IGNORE_ELEM(I, J) IGNORE(I), IGNORE(J)
#define DOT(X, I1, I2, I3) I1.X, I2.X, I3.X /* NOLINT(bugprone-macro-parentheses) */
#define SUBMAT_(I1, I2, I3, J1, J2, J3) DOT(I1, J1, J2, J3), DOT(I2, J1, J2, J3), DOT(I3, J1, J2, J3)
#define SUBMAT(...) STDU_UNARY(SUBMAT_(__VA_ARGS__))
#define ADJUGATE_ELEM(I, J) (DET3(SUBMAT(IGNORE_ELEM(J, I))))
#define ADJUGATE_VEC(U) { ADJUGATE_ELEM(x, U), ADJUGATE_ELEM(y, U), ADJUGATE_ELEM(z, U), ADJUGATE_ELEM(w, U) }
    
    mat4x4 mat4x4::adjugate() const {
        return { ADJUGATE_VEC(x), ADJUGATE_VEC(y), ADJUGATE_VEC(z), ADJUGATE_VEC(w) };
    }

    mat4x4 mat4x4::inv() const {
        return adjugate() * (1 / det());
    }

#undef IGNORE
#undef IGNORE_1
#undef IGNORE_2
#undef IGNORE_3
#undef IGNORE_4
#undef IGNORE_ELEM
#undef DOT
#undef SUBMAT_
#undef SUBMAT
#undef ADJUGATE_ELEM
#undef ADJUGATE_VEC

#undef DET3_
#undef DET3
#undef DETMAT3

    mat4x4 mat4x4::unit_axis(Direction3D xd, Direction3D yd, Direction3D zd) {
        return { fvec3(xd).with_w(0), fvec3(yd).with_w(0), fvec3(zd).with_w(0),};
    }

#define VECADD(U) x.U * v.x + y.U * v.y + z.U * v.z + w.U /* NOLINT(bugprone-macro-parentheses) */
    fvec3 mat4x4::operator*(const fvec3& v) const {
        return { VECADD(x), VECADD(y), VECADD(z) };
    }

    fvec4 mat4x4::operator*(const fvec4& v) const {
        return { VECADD(x) * v.w,   // * NOTE! this might break as this uses
                 VECADD(y) * v.w,   // * the fact that VECADD is not inclosed
                 VECADD(z) * v.w,   // * in brackets. expand and you'll see  
                 VECADD(w) * v.w }; // * how it hacks the compiler
    }
#undef VECADD
    
    mat4x4 mat4x4::operator*(const mat4x4& m) const {
        return { *this * m.x, *this * m.y, *this * m.z, *this * m.w };
    }

    mat4x4 mat4x4::operator*(float s) const {
        return { x * s, y * s, z * s, w * s };
    }

    std::ostream& operator<<(std::ostream& stream, const mat4x4& mat) {
        stream << std::fixed << std::setprecision(3);
        stream << '[' << mat.x.x << ',' << mat.y.x << ',' << mat.z.x << ',' << mat.w.x << ']' << '\n';
        stream << '[' << mat.x.y << ',' << mat.y.y << ',' << mat.z.y << ',' << mat.w.y << ']' << '\n';
        stream << '[' << mat.x.z << ',' << mat.y.z << ',' << mat.z.z << ',' << mat.w.z << ']' << '\n';
        stream << '[' << mat.x.w << ',' << mat.y.w << ',' << mat.z.w << ',' << mat.w.w << ']';// << '\n'
        stream << std::setprecision(-1);
        return stream;
    }
}
