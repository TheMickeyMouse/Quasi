#pragma once

#include "Math/Complex.h"
#include "Math/Geometry.h"
#include "Math/Transform2D.h"

namespace Quasi::Physics2D {
    using namespace Math;

    struct PhysicsTransform { // different from math::transform2d, as this has no scale
        fv2 position;
        Rotation2D rotation;

        PhysicsTransform(const fv2& pos = {}, const Rotation2D& rot = {}) : position(pos), rotation(rot) {}

        static PhysicsTransform Translate(const fv2& p);
        static PhysicsTransform Rotation(const Rotation2D& q);

        fv2 Transform(const fv2& point) const;
        fv2 TransformDir(const fv2& dir) const;
        fv2 TransformInverse(const fv2& point) const;
        fv2 TransformInverseDir(const fv2& dir) const;
        fLine2D TransformLine(const fLine2D& line) const;
        fRect2D TransformRect(const fRect2D& rect) const;

        Transform2D AsObjectTransform(const fv2& scale = 1) const;

        PhysicsTransform Inverse() const;

        PhysicsTransform Applied(const PhysicsTransform& transformer) const; // apply transform onto self
        PhysicsTransform AppliedTo(const PhysicsTransform& transformed) const { return transformed.Applied(*this); }
        PhysicsTransform& Apply(const PhysicsTransform& transformer); // apply transform onto self
        void ApplyTo(PhysicsTransform& transformed) const { transformed.Apply(*this); }

        PhysicsTransform Then(const PhysicsTransform& t) const { return AppliedTo(t); }

        void Reset();

        fv2 operator*(const fv2& p) const { return Transform(p); }
        fRect2D  operator*(const fRect2D& r) const { return TransformRect(r); }
        PhysicsTransform operator*(const PhysicsTransform& t) const { return Applied(t); }
        PhysicsTransform& operator*=(const PhysicsTransform& t) { return Apply(t); }
    };
} // Physics2D
