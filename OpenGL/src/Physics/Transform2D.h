#pragma once

#include "Rect.h"
#include "Vector.h"

namespace Physics2D {
    struct Transform {
        Maths::fvec2 position, rotation = { 1, 0 };

        Transform(const Maths::fvec2& pos = {}, const Maths::fvec2& rot = { 1, 0 }) : position(pos), rotation(rot) {}

        void SetAngle(float angle);
        static Maths::fvec2 RotationMul(const Maths::fvec2& a, const Maths::fvec2& b);
        [[nodiscard]] Transform Inverse() const;
        [[nodiscard]] Transform operator*(const Transform& t) const;
        [[nodiscard]] Maths::fvec2 operator*(const Maths::fvec2& p) const;
        [[nodiscard]] Maths::rect2f operator*(const Maths::rect2f& r) const;
    };
} // Physics2D
