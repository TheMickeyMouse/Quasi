#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    // ! Important ! assumes points are in anti-clockwise order
    template <u32 N>
    class PolygonShape : public IShape {
    public:
        static constexpr bool DYNAMIC = N == 0;
        std::conditional_t<DYNAMIC, Vec<Math::fVector2>, Array<Math::fVector2, N>> points;

        PolygonShape() = default;
        PolygonShape(UncheckedMarker, decltype(points)&& ps) : points(std::move(ps)) {}
        PolygonShape(decltype(points)&& ps);
        ~PolygonShape() = default;

        [[nodiscard]] i32 Size() const;
        void AddPoint(const Math::fVector2& p) requires DYNAMIC;
        void AddPoint(const Math::fVector2& p, u32 i) requires DYNAMIC;
        void RemovePoint(u32 i) requires DYNAMIC;
        void PopPoint() requires DYNAMIC;

        [[nodiscard]] const Math::fVector2& PointAt(i32 i) const { return points[(i % Size() + Size()) % Size()]; }
        Math::fVector2& PointAt(i32 i) { return points[(i % Size() + Size()) % Size()]; }
        [[nodiscard]] float Xof(i32 i) const { return PointAt(i).x; }
        [[nodiscard]] float Yof(i32 i) const { return PointAt(i).y; }

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const;

        [[nodiscard]] PolygonShape Transform(const PhysicsTransform& xf) const;

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };

    using TriangleShape = PolygonShape<3>;
    using QuadShape = PolygonShape<4>;
    using DynPolyShape = PolygonShape<0>;
} // Quasi
