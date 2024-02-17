#pragma once
#include <ostream>
#include "opengl.h"
#include "Vector.h"

namespace Maths {
    // column major mat
    class mat4x4 {
        fvec4 x, y, z, w;
        
    public:
        mat4x4(const fvec4& _x = { 1.0f, 0.0f, 0.0f, 0.0f },
               const fvec4& _y = { 0.0f, 1.0f, 0.0f, 0.0f },
               const fvec4& _z = { 0.0f, 0.0f, 1.0f, 0.0f },
               const fvec4& _w = { 0.0f, 0.0f, 0.0f, 1.0f })
        : x(_x), y(_y), z(_z), w(_w) {}

        OPENGL_API const float* get_in_col() const;
        OPENGL_API const fvec4* get_cols() const;

        OPENGL_API void translate(const fvec3& translation);
        OPENGL_API static mat4x4 translate_mat(const fvec3& translation, const mat4x4& mat = {});
        OPENGL_API void scale(const fvec3& scale);
        OPENGL_API static mat4x4 scale_mat(const fvec3& scale, const mat4x4& mat = {});
        OPENGL_API void rotate(const fvec3& rotation);
        OPENGL_API static mat4x4 rotate_mat(const fvec3& rotation, const mat4x4& mat = {});
        OPENGL_API static mat4x4 rotate_identity(const fvec3& rotation);
        OPENGL_API static mat4x4 rotate_x(float roll);
        OPENGL_API static mat4x4 rotate_y(float pitch);
        OPENGL_API static mat4x4 rotate_z(float yaw);

        OPENGL_API static mat4x4 ortho_projection(const rect3f& box);
        OPENGL_API static mat4x4 perspective_projection(const rect3f& box);
        OPENGL_API static mat4x4 perspective_fov(float fovDeg, float aspect, float near, float far);
        OPENGL_API static mat4x4 transform(const fvec3& translate, const fvec3& scale, const fvec3& rotate);

        OPENGL_API [[nodiscard]] mat4x4 transpose() const;
        OPENGL_API [[nodiscard]] float det() const; // determinate
        OPENGL_API [[nodiscard]] mat4x4 adjugate() const;
        OPENGL_API [[nodiscard]] mat4x4 inv() const; // inverse

        OPENGL_API static mat4x4 unit_axis(Direction3D xd, Direction3D yd, Direction3D zd);

        OPENGL_API float operator[](unsigned i) { return *((float*)this + i); }

        OPENGL_API fvec3 operator* (const fvec3& v) const;
        OPENGL_API fvec4 operator* (const fvec4& v) const;
        OPENGL_API mat4x4 operator* (const mat4x4& m) const;
        OPENGL_API mat4x4 operator* (float s) const;
        mat4x4& operator*=(const mat4x4& m) { *this = *this * m; return *this; }
        
        friend OPENGL_API std::ostream& operator<<(std::ostream& stream, const mat4x4& mat);
    };

    using mat3D = mat4x4;

    OPENGL_API std::ostream& operator<<(std::ostream& stream, const mat4x4& mat);
}