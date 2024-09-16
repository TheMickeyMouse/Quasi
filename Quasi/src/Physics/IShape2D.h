#pragma once
#include "Geometry.h"
#include "PhysicsTransform2D.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;
    struct Manifold;

    class IShape {
    public:
        [[nodiscard]] float ComputeArea() const = delete;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const = delete;
        [[nodiscard]] Math::fVector2 CenterOfMass() const = delete;

        // SAT helpers
        enum ClipPrimitive { PRIM_CIRCLE, PRIM_POINT, PRIM_EDGE };
        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const = delete;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const = delete;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const { return { {}, {} }; }
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const = delete;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const = delete;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const = delete;
    };
}
