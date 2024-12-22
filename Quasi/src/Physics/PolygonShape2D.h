#pragma once
#include "IShape2D.h"

namespace Quasi::Physics2D {
    template <class T>
    class BasicPolygonShape : public IShape { // removes repeating code, like area calc/length updating
    public:
        u32 Size() const { return ((const T*)this)->Size(); }

        i32 WrapIndex(i32 i) const { return (i % (i32)Size() + (i32)Size()) % (i32)Size(); }

        const fVector2& PointAt(i32 i) const { return ((const T*)this)->PointAt(i); }
        const fVector2& PointAtWrap(i32 i) const { return PointAt(WrapIndex(i)); }
        fVector2& PointAt(i32 i) { return ((T*)this)->PointAt(i); }
        fVector2& PointAtWrap(i32 i) { return PointAt(WrapIndex(i)); }

        float InvLenBtwn(i32 i) const { return ((const T*)this)->InvLenBtwn(i); }
        float InvLenBtwnWrap(i32 i) const { return InvLenBtwn(WrapIndex(i)); }
        float& InvLenBtwn(i32 i) { return ((T*)this)->InvLenBtwn(i); }
        float& InvLenBtwnWrap(i32 i) { return InvLenBtwn(WrapIndex(i)); }
        void UpdateLenBtwn(i32 i) { InvLenBtwn(i) = 1 / PointAt(i).dist(PointAtWrap(i + 1)); }
        void UpdateLenBtwnWrap(i32 i) { InvLenBtwnWrap(i) = 1 / PointAtWrap(i).dist(PointAtWrap(i + 1)); }

        void FixPolygon();
        void FixCenterOfMass();

        float ComputeArea() const;
        fRect2D ComputeBoundingBox() const;
        fVector2 CenterOfMass() const;
        float Inertia() const;

        fVector2 NearestPointTo(const fVector2& point) const;
        fVector2 FurthestAlong(const fVector2& normal) const;
        fLine2D BestEdgeFor(const fVector2& normal) const;
        fRange ProjectOntoAxis(const fVector2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };

    template <u32 N>
    class StaticPolygonShape : public BasicPolygonShape<StaticPolygonShape<N>> {
    public:
        fVector2 points[N];
        float invDists[N];

        StaticPolygonShape() = default;
        StaticPolygonShape(Span<const fVector2> ps);

        u32 Size() const { return N; }

        const fVector2& PointAt(i32 i) const { return points[i]; }
        fVector2& PointAt(i32 i) { return points[i]; }
        float InvLenBtwn(i32 i) const { return invDists[i]; }
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
        DynPolygonShape(const DynPolygonShape& s) : data(s.data.Clone()) {}
        DynPolygonShape& operator=(const DynPolygonShape& s) { data = s.data.Clone(); return *this; }

        u32 Size() const;
        void AddPoint(const fVector2& p);
        void AddPoint(const fVector2& p, u32 i);
        void RemovePoint(u32 i);
        void PopPoint();
        void SetPoint(const fVector2& p, i32 i);
        void SetPointWrap(const fVector2& p, i32 i) { SetPoint(p, WrapIndex(i)); }

        const PointWithInvDist& DataAt(i32 i) const { return data[i]; }
        PointWithInvDist& DataAt(i32 i) { return data[i]; }

        const fVector2& PointAt(i32 i) const { return DataAt(i).coords; }
        fVector2& PointAt(i32 i) { return DataAt(i).coords; }
        float InvLenBtwn(i32 i) const { return DataAt(i).invDist; }
        float& InvLenBtwn(i32 i) { return DataAt(i).invDist; }
    };
} // Quasi
