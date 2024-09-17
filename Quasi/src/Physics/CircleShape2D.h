#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class TransformedCircleShape;

    class CircleShape : public IShape {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return 0; }

        [[nodiscard]] TransformedCircleShape Transform(const PhysicsTransform& xf) const;
    };

    class TransformedCircleShape : public ITransformedShape {
    public:
        Math::fVector2 center;
        float radius = 1.0f;

        TransformedCircleShape(const Math::fVector2& center, float r) : center(center), radius(r) {}
        TransformedCircleShape(const CircleShape& c, const PhysicsTransform& xf = {})
            : TransformedCircleShape(c.Transform(xf)) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return center; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const { return false; }
    };
} // Quasi
