#pragma once
#include "core.h"
#include "Rect.h"
#include "Vector.h"

namespace Physics2D {
    struct Transform {
        Maths::fvec2 position, rotation = { 1, 0 };

        Transform(const Maths::fvec2& pos = {}, const Maths::fvec2& rot = { 1, 0 }) : position(pos), rotation(rot) {}

        OPENGL_API void SetAngle(float angle);
        OPENGL_API static Maths::fvec2 RotationMul(const Maths::fvec2& a, const Maths::fvec2& b);
        [[nodiscard]] OPENGL_API Transform Inverse() const;
        [[nodiscard]] OPENGL_API Transform operator*(const Transform& t) const;
        [[nodiscard]] OPENGL_API Maths::fvec2 operator*(const Maths::fvec2& p) const;
        [[nodiscard]] OPENGL_API Maths::rect2f operator*(const Maths::rect2f& r) const;
    };
} // Physics2D
