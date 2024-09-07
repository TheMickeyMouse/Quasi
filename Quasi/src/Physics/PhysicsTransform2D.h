#pragma once

#include "Complex.h"
#include "Rect.h"

namespace Quasi::Physics2D {
    struct PhysicsTransform { // different from math::transform2d, as this has no scale
        Math::fVector2 position;
        Math::fComplex rotation;

        PhysicsTransform(const Math::fVector2& pos = {}, const Math::fComplex& rot = 1) : position(pos), rotation(rot) {}

        void Translate(const Math::fVector2& p);
        void Rotate(float r);
        void Rotate(const Math::fComplex& r);
        [[nodiscard]] PhysicsTransform Translated(const Math::fVector2& p) const;
        [[nodiscard]] PhysicsTransform Rotated(float r) const;
        [[nodiscard]] PhysicsTransform Rotated(const Math::fComplex& r) const;

        static PhysicsTransform Translation(const Math::fVector2& p);
        static PhysicsTransform Rotation(float r);
        static PhysicsTransform Rotation(const Math::fComplex& q);

        [[nodiscard]] Math::fVector2 Transform       (const Math::fVector2&  point) const;
        [[nodiscard]] Math::fVector2 TransformOffset (const Math::fVector2& offset) const;
                      void           TransformInplace(Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 TransformInverse       (const Math::fVector2&  point) const;
                      void           TransformInverseInplace(Math::fVector2& point) const;
        [[nodiscard]] Math::fRect2D  TransformRect(const Math::fRect2D& rect) const;

        [[nodiscard]] Math::Transform2D AsObjectTransform(const Math::fVector2& scale = 1) const;

        [[nodiscard]] PhysicsTransform Inverse() const;

        [[nodiscard]] PhysicsTransform Applied(const PhysicsTransform& transformer) const; // apply transform onto self
        [[nodiscard]] PhysicsTransform AppliedTo(const PhysicsTransform& transformed) const { return transformed.Applied(*this); }
        PhysicsTransform& Apply(const PhysicsTransform& transformer); // apply transform onto self
        void ApplyTo(PhysicsTransform& transformed) const { transformed.Apply(*this); }

        [[nodiscard]] PhysicsTransform Then(const PhysicsTransform& t) const { return AppliedTo(t); }

        void Reset();

        [[nodiscard]] Math::fVector2 operator*(const Math::fVector2& p) const { return Transform(p); }
        [[nodiscard]] Math::fRect2D  operator*(const Math::fRect2D& r) const { return TransformRect(r); }
        [[nodiscard]] PhysicsTransform operator*(const PhysicsTransform& t) const { return Applied(t); }
        PhysicsTransform& operator*=(const PhysicsTransform& t) { return Apply(t); }
    };
} // Physics2D
