#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class CircleShape : public IShape {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] fRect2D ComputeBoundingBox() const;
        [[nodiscard]] fVector2 CenterOfMass() const { return 0; }
        [[nodiscard]] float Inertia() const;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const;
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const { return false; }
    };
} // Quasi
