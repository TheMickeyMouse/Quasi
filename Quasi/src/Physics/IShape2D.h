#pragma once
#include "Geometry.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"
#include "PhysicsTransform2D.h"
#include "Math/Geometry.h"
#include "Math/Rect.h"
#include "Math/Vector.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;
    struct Manifold;

    class IShape {
    public:
        float ComputeArea() const = delete;
        fRect2D ComputeBoundingBox() const = delete;

        fVector2 NearestPointTo(const fVector2& point) const = delete;
        fVector2 FurthestAlong(const fVector2& normal) const = delete;
        fLine2D BestEdgeFor(const fVector2& normal) const { return { {}, {} }; }
        fRange ProjectOntoAxis(const fVector2& axis) const = delete;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const = delete;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const = delete;

        void UpdateTransform(const PhysicsTransform& xf) = delete;

        enum ClipPrimitive { PRIM_CIRCLE, PRIM_LINE, PRIM_POLYGON };
        enum Type { CIRCLE, CAPSULE, RECT, TRI, QUAD, POLY };
    };
}
