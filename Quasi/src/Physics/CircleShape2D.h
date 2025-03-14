#pragma once
#include "IShape2D.h"
#include "Math/Rect.h"

namespace Quasi::Physics2D {
    class CircleShape : public IShape {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        float ComputeArea() const;
        fRect2D ComputeBoundingBox() const;
        float Inertia() const;

        fVector2 NearestPointTo(const fVector2& point) const;
        fVector2 FurthestAlong(const fVector2& normal) const;
        fRange ProjectOntoAxis(const fVector2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const { return false; }
    };
} // Quasi
