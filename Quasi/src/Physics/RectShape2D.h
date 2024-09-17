#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class TransformedRectShape;

    class RectShape : public IShape {
    public:
        float hx, hy;

        RectShape(float x, float y) : hx(x), hy(y) {}

        [[nodiscard]] Math::fVector2 Corner(bool px, bool py) const;

        [[nodiscard]] float ComputeArea() const { return 4 * hx * hy; }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const { return { { -hx, -hy }, { hx, hy } }; }
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return 0; }

        [[nodiscard]] TransformedRectShape Transform(const PhysicsTransform& xf) const;
    };

    class TransformedRectShape : public ITransformedShape {
    public:
        Math::fVector2 center, x, y;

        TransformedRectShape(const Math::fVector2& c, const Math::fVector2& x, const Math::fVector2& y) : center(c), x(x), y(y) {}
        TransformedRectShape(const RectShape& r, const PhysicsTransform& xf = {})
            : TransformedRectShape(r.Transform(xf)) {}

        [[nodiscard]] Math::fVector2 Corner(bool px, bool py) const;

        [[nodiscard]] float ComputeArea() const { return 4 * x.len() * y.len(); }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return center; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };
} // Quasi
