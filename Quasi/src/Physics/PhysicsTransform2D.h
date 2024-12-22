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
        PhysicsTransform Translated(const fVector2& p) const;
        PhysicsTransform Rotated(float r) const;
        PhysicsTransform Rotated(const fComplex& r) const;

        static PhysicsTransform Translation(const fVector2& p);
        static PhysicsTransform Rotation(float r);
        static PhysicsTransform Rotation(const fComplex& q);

        fVector2 Transform(const fVector2& point) const;
        fVector2 TransformDir(const fVector2& dir) const;
                      void     TransformInplace(fVector2& point) const;
                      void     TransformInplaceDir(fVector2& dir) const;
        fVector2 TransformInverse(const fVector2& point) const;
        fVector2 TransformInverseDir(const fVector2& dir) const;
                      void     TransformInverseInplace(fVector2& point) const;
                      void     TransformInverseInplaceDir(fVector2& dir) const;
        fLine2D  TransformLine(const fLine2D& line) const;
        fRect2D  TransformRect(const fRect2D& rect) const;

        Transform2D AsObjectTransform(const fVector2& scale = 1) const;

        PhysicsTransform Inverse() const;

        PhysicsTransform Applied(const PhysicsTransform& transformer) const; // apply transform onto self
        PhysicsTransform AppliedTo(const PhysicsTransform& transformed) const { return transformed.Applied(*this); }
        PhysicsTransform& Apply(const PhysicsTransform& transformer); // apply transform onto self
        void ApplyTo(PhysicsTransform& transformed) const { transformed.Apply(*this); }

        PhysicsTransform Then(const PhysicsTransform& t) const { return AppliedTo(t); }

        void Reset();

        fVector2 operator*(const fVector2& p) const { return Transform(p); }
        fRect2D  operator*(const fRect2D& r) const { return TransformRect(r); }
        PhysicsTransform operator*(const PhysicsTransform& t) const { return Applied(t); }
        PhysicsTransform& operator*=(const PhysicsTransform& t) { return Apply(t); }
    };
} // Physics2D
