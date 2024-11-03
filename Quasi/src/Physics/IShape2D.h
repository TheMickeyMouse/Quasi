#pragma once
#include "Geometry.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"
#include "PhysicsTransform2D.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;
    struct Manifold;

    class IShape {
    public:
        [[nodiscard]] float ComputeArea() const = delete;
        [[nodiscard]] fRect2D ComputeBoundingBox() const = delete;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const = delete;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const = delete;
        [[nodiscard]] fLine2D BestEdgeFor(const fVector2& normal) const { return { {}, {} }; }
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const = delete;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const = delete;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const = delete;

        void UpdateTransform(const PhysicsTransform& xf) = delete;

        enum ClipPrimitive { PRIM_CIRCLE, PRIM_LINE, PRIM_POLYGON };
        enum Type { CIRCLE, CAPSULE, RECT, TRI, QUAD, POLY };
    };
}
