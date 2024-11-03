#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    template <class T>
    class BasicPolygonShape : public IShape { // removes repeating code, like area calc/length updating
    public:
        [[nodiscard]] u32 Size() const { return ((const T*)this)->Size(); }

        [[nodiscard]] i32 WrapIndex(i32 i) const { return (i % (i32)Size() + (i32)Size()) % (i32)Size(); }

        [[nodiscard]] const fVector2& PointAt(i32 i) const { return ((const T*)this)->PointAt(i); }
        [[nodiscard]] const fVector2& PointAtWrap(i32 i) const { return PointAt(WrapIndex(i)); }
        fVector2& PointAt(i32 i) { return ((T*)this)->PointAt(i); }
        fVector2& PointAtWrap(i32 i) { return PointAt(WrapIndex(i)); }

        [[nodiscard]] float InvLenBtwn(i32 i) const { return ((const T*)this)->InvLenBtwn(i); }
        [[nodiscard]] float InvLenBtwnWrap(i32 i) const { return InvLenBtwn(WrapIndex(i)); }
        float& InvLenBtwn(i32 i) { return ((T*)this)->InvLenBtwn(i); }
        float& InvLenBtwnWrap(i32 i) { return InvLenBtwn(WrapIndex(i)); }
        void UpdateLenBtwn(i32 i) { InvLenBtwn(i) = 1 / PointAt(i).dist(PointAtWrap(i + 1)); }
        void UpdateLenBtwnWrap(i32 i) { InvLenBtwnWrap(i) = 1 / PointAtWrap(i).dist(PointAtWrap(i + 1)); }

        void FixPolygon();
        void FixCenterOfMass();

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] fRect2D ComputeBoundingBox() const;
        [[nodiscard]] fVector2 CenterOfMass() const;
        [[nodiscard]] float Inertia() const;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const;
        [[nodiscard]] fLine2D BestEdgeFor(const fVector2& normal) const;
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };

    template <u32 N>
    class StaticPolygonShape : public BasicPolygonShape<StaticPolygonShape<N>> {
    public:
        fVector2 points[N];
        float invDists[N];

        StaticPolygonShape() = default;
        StaticPolygonShape(Span<const fVector2, N> ps);

        [[nodiscard]] u32 Size() const { return N; }

        [[nodiscard]] const fVector2& PointAt(i32 i) const { return points[i]; }
        fVector2& PointAt(i32 i) { return points[i]; }
        [[nodiscard]] float InvLenBtwn(i32 i) const { return invDists[i]; }
        float& InvLenBtwn(i32 i) { return invDists[i]; }
    };

    using TriangleShape = StaticPolygonShape<3>;
    using QuadShape     = StaticPolygonShape<4>;

    class DynPolygonShape : public BasicPolygonShape<DynPolygonShape> {
    public:
        struct PointWithInvDist {
            fVector2 coords;
            float invDist;
        };

        Vec<PointWithInvDist> data;

    private:
        DynPolygonShape(Vec<PointWithInvDist>&& d) : data(std::move(d)) {}
    public:
        DynPolygonShape() = default;
        DynPolygonShape(Span<const fVector2> points);

        [[nodiscard]] u32 Size() const;
        void AddPoint(const fVector2& p);
        void AddPoint(const fVector2& p, u32 i);
        void RemovePoint(u32 i);
        void PopPoint();
        void SetPoint(const fVector2& p, i32 i);
        void SetPointWrap(const fVector2& p, i32 i) { SetPoint(p, WrapIndex(i)); }

        [[nodiscard]] const PointWithInvDist& DataAt(i32 i) const { return data[i]; }
        PointWithInvDist& DataAt(i32 i) { return data[i]; }

        [[nodiscard]] const fVector2& PointAt(i32 i) const { return DataAt(i).coords; }
        fVector2& PointAt(i32 i) { return DataAt(i).coords; }
        [[nodiscard]] float InvLenBtwn(i32 i) const { return DataAt(i).invDist; }
        float& InvLenBtwn(i32 i) { return DataAt(i).invDist; }
    };
} // Quasi
