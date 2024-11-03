#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class RectShape : public IShape {
    public:
        float hx, hy;

        RectShape(float x, float y) : hx(x), hy(y) {}

        [[nodiscard]] fVector2 Corner(bool px, bool py) const;

        [[nodiscard]] float ComputeArea() const { return 4 * hx * hy; }
        [[nodiscard]] fRect2D ComputeBoundingBox() const;
        [[nodiscard]] float Inertia() const;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const;
        [[nodiscard]] fLine2D BestEdgeFor(const fVector2& normal) const;
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };
} // Quasi
