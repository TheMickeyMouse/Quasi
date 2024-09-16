#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    template <u32 N>
    class PolygonShape : public IShape {
    public:
        static constexpr bool DYNAMIC = N == 0;
        std::conditional_t<DYNAMIC, Vec<Math::fVector2>, Array<Math::fVector2, N>> points;

        PolygonShape(decltype(points)&& ps) : points(std::move(ps)) {}
        template <class... Ts>
        PolygonShape(const Ts& ...ps) : points { ps... } {}
        ~PolygonShape() = default;

        [[nodiscard]] u32 Size() const;
        void AddPoint(const Math::fVector2& p) requires DYNAMIC;
        void AddPoint(const Math::fVector2& p, u32 i) requires DYNAMIC;
        void RemovePoint(u32 i) requires DYNAMIC;
        void PopPoint() requires DYNAMIC;

        [[nodiscard]] const Math::fVector2& PointAt(u32 i) const { return points[i % Size()]; }
        Math::fVector2& PointAt(u32 i) { return points[i % Size()]; }
        [[nodiscard]] float Xof(u32 i) const { return PointAt(i).x; }
        [[nodiscard]] float Yof(u32 i) const { return PointAt(i).y; }

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const;

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const;
    };

    using TriangleShape = PolygonShape<3>;
    using QuadShape = PolygonShape<4>;
    using DynPolyShape = PolygonShape<0>;
} // Quasi
