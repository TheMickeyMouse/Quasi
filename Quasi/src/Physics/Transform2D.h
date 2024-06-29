#pragma once

#include "Rect.h"

namespace Quasi::Physics2D {
    struct Transform {
        Math::fVector2 position, rotation = { 1, 0 };

        Transform(const Math::fVector2& pos = {}, const Math::fVector2& rot = { 1, 0 }) : position(pos), rotation(rot) {}

        void SetAngle(float angle);
        static Math::fVector2 RotationMul(const Math::fVector2& a, const Math::fVector2& b);
        [[nodiscard]] Transform Inverse() const;
        [[nodiscard]] Transform operator*(const Transform& t) const;
        [[nodiscard]] Math::fVector2 operator*(const Math::fVector2& p) const;
        [[nodiscard]] Math::fRect2D operator*(const Math::fRect2D& r) const;
    };
} // Physics2D
