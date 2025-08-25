#pragma once
#include "Utils/Math/Geometry.h"
#include "Utils/Math/Rect.h"
#include "PhysicsTransform2D.h"
#include "Utils/Math/Vector.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;
    struct Manifold;

    class IShape {
    public:
        float ComputeArea() const = delete;
        fRect2D ComputeBoundingBox() const = delete;

        fv2 NearestPointTo(const fv2& point) const = delete;
        fv2 FurthestAlong(const fv2& normal) const = delete;
        fLine2D BestEdgeFor(const fv2& /* normal */) const { return { {}, {} }; }
        fRange ProjectOntoAxis(const fv2& axis) const = delete;
        fRange ProjectOntoOwnAxis(u32 axisID, const fv2& axis) const = delete;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const = delete;

        void UpdateTransform(const PhysicsTransform& xf) = delete;

        enum ClipPrimitive { PRIM_CIRCLE, PRIM_LINE, PRIM_POLYGON };
        enum Type { CIRCLE, CAPSULE, RECT, POLY_SMALL, POLY };
    };
}
