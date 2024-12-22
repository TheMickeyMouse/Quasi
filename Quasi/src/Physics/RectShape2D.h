#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class RectShape : public IShape {
    public:
        float hx, hy;

        RectShape(float x, float y) : hx(x), hy(y) {}

        fVector2 Corner(bool px, bool py) const;

        float ComputeArea() const { return 4 * hx * hy; }
        fRect2D ComputeBoundingBox() const;
        float Inertia() const;

        fVector2 NearestPointTo(const fVector2& point) const;
        fVector2 FurthestAlong(const fVector2& normal) const;
        fLine2D BestEdgeFor(const fVector2& normal) const;
        fRange ProjectOntoAxis(const fVector2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };
} // Quasi
