#pragma once

#include "Complex.h"
#include "Geometry.h"
#include "Rect.h"

namespace Quasi::Physics2D {
    using namespace Math;

    struct PhysicsTransform { // different from math::transform2d, as this has no scale
        fVector2 position;
        fComplex rotation;

        PhysicsTransform(const fVector2& pos = {}, const fComplex& rot = 1) : position(pos), rotation(rot) {}

        void Translate(const fVector2& p);
        void Rotate(float r);
        void Rotate(const fComplex& r);
        [[nodiscard]] PhysicsTransform Translated(const fVector2& p) const;
        [[nodiscard]] PhysicsTransform Rotated(float r) const;
        [[nodiscard]] PhysicsTransform Rotated(const fComplex& r) const;

        static PhysicsTransform Translation(const fVector2& p);
        static PhysicsTransform Rotation(float r);
        static PhysicsTransform Rotation(const fComplex& q);

        [[nodiscard]] fVector2 Transform       (const fVector2& point) const;
        [[nodiscard]] fVector2 TransformOffset (const fVector2& offset) const;
                      void     TransformInplace(fVector2& point) const;
        [[nodiscard]] fVector2 TransformInverse       (const fVector2& point) const;
        [[nodiscard]] fVector2 TransformInverseOffset (const fVector2& offset) const;
                      void     TransformInverseInplace(fVector2& point) const;
        [[nodiscard]] fLine2D  TransformLine(const fLine2D& line) const;
        [[nodiscard]] fRect2D  TransformRect(const fRect2D& rect) const;

        [[nodiscard]] Transform2D AsObjectTransform(const fVector2& scale = 1) const;

        [[nodiscard]] PhysicsTransform Inverse() const;

        [[nodiscard]] PhysicsTransform Applied(const PhysicsTransform& transformer) const; // apply transform onto self
        [[nodiscard]] PhysicsTransform AppliedTo(const PhysicsTransform& transformed) const { return transformed.Applied(*this); }
        PhysicsTransform& Apply(const PhysicsTransform& transformer); // apply transform onto self
        void ApplyTo(PhysicsTransform& transformed) const { transformed.Apply(*this); }

        [[nodiscard]] PhysicsTransform Then(const PhysicsTransform& t) const { return AppliedTo(t); }

        void Reset();

        [[nodiscard]] fVector2 operator*(const fVector2& p) const { return Transform(p); }
        [[nodiscard]] fRect2D  operator*(const fRect2D& r) const { return TransformRect(r); }
        [[nodiscard]] PhysicsTransform operator*(const PhysicsTransform& t) const { return Applied(t); }
        PhysicsTransform& operator*=(const PhysicsTransform& t) { return Apply(t); }
    };
} // Physics2D
