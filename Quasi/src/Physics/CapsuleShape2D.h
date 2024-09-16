#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    class CapsuleShape : public IShape {
    public:
        Math::fVector2 forward;
        float radius = 0.0f;

        CapsuleShape(const Math::fVector2& fwd, float r) : forward(fwd), radius(r) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const { return forward * 0.5f; }
        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const;
    };
} // Quasi
