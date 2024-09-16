#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class RectShape : public IShape {
    public:
        float hx, hy;

        RectShape(float x, float y) : hx(x), hy(y) {}

        [[nodiscard]] Math::fVector2 Corner(bool px, bool py) const;

        [[nodiscard]] float ComputeArea() const { return 4 * hx * hy; }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const { return { { -hx, -hy }, { hx, hy } }; }
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return 0; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const;
    };
} // Quasi
