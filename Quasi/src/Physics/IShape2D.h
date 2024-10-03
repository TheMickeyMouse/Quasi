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
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const = delete;
        [[nodiscard]] Math::fVector2 CenterOfMass() const = delete;

        enum ClipPrimitive { PRIM_CIRCLE, PRIM_LINE, PRIM_POLYGON };
        enum Type { CIRCLE, CAPSULE, RECT, TRI, QUAD, POLY };
    };

    class ITransformedShape : public IShape {
    public:
        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const = delete;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const = delete;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal) const { return { {}, {} }; }
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const = delete;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const = delete;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const = delete;
    };
}
