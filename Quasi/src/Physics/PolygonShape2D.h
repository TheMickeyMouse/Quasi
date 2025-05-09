#pragma once
#include "IShape2D.h"
#include "Utils/Vec.h"

namespace Quasi::Physics2D {
    template <class T>
    class BasicPolygonShape : public IShape { // removes repeating code, like area calc/length updating
    public:
        u32 Size() const { return ((const T*)this)->Size(); }

        i32 WrapIndex(i32 i) const { return (i % (i32)Size() + (i32)Size()) % (i32)Size(); }

        const fv2& PointAt(i32 i) const { return ((const T*)this)->PointAt(i); }
        const fv2& PointAtWrap(i32 i) const { return PointAt(WrapIndex(i)); }
        fv2& PointAt(i32 i) { return ((T*)this)->PointAt(i); }
        fv2& PointAtWrap(i32 i) { return PointAt(WrapIndex(i)); }

        float InvLenBtwn(i32 i) const { return ((const T*)this)->InvLenBtwn(i); }
        float InvLenBtwnWrap(i32 i) const { return InvLenBtwn(WrapIndex(i)); }
        float& InvLenBtwn(i32 i) { return ((T*)this)->InvLenBtwn(i); }
        float& InvLenBtwnWrap(i32 i) { return InvLenBtwn(WrapIndex(i)); }
        void UpdateLenBtwn(i32 i) { InvLenBtwn(i) = 1 / PointAt(i).Dist(PointAtWrap(i + 1)); }
        void UpdateLenBtwnWrap(i32 i) { InvLenBtwnWrap(i) = 1 / PointAtWrap(i).Dist(PointAtWrap(i + 1)); }

        void FixPolygon();
        void FixCenterOfMass();

        float ComputeArea() const;
        fRect2D ComputeBoundingBox() const;
        fv2 CenterOfMass() const;
        float Inertia() const;

        fv2 NearestPointTo(const fv2& point) const;
        fv2 FurthestAlong(const fv2& normal) const;
        fLine2D BestEdgeFor(const fv2& normal) const;
        fRange ProjectOntoAxis(const fv2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fv2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };

    template <u32 N>
    class StaticPolygonShape : public BasicPolygonShape<StaticPolygonShape<N>> {
    public:
        fv2 points[N];
        float invDists[N];

        StaticPolygonShape() = default;
        StaticPolygonShape(Span<const fv2> ps);

        u32 Size() const { return N; }

        const fv2& PointAt(i32 i) const { return points[i]; }
        fv2& PointAt(i32 i) { return points[i]; }
        float InvLenBtwn(i32 i) const { return invDists[i]; }
        float& InvLenBtwn(i32 i) { return invDists[i]; }
    };

    using TriangleShape = StaticPolygonShape<3>;
    using QuadShape     = StaticPolygonShape<4>;

    class DynPolygonShape : public BasicPolygonShape<DynPolygonShape> {
    public:
        struct PointWithInvDist {
            fv2 coords;
            float invDist;
        };

        Vec<PointWithInvDist> data;
    private:
        DynPolygonShape(Vec<PointWithInvDist>&& d) : data(std::move(d)) {}
    public:
        DynPolygonShape() = default;
        DynPolygonShape(Span<const fv2> points);
        DynPolygonShape(const DynPolygonShape& s) : data(s.data.Clone()) {}
        DynPolygonShape& operator=(const DynPolygonShape& s) { data = s.data.Clone(); return *this; }

        u32 Size() const;
        void AddPoint(const fv2& p);
        void AddPoint(const fv2& p, u32 i);
        void RemovePoint(u32 i);
        void PopPoint();
        void SetPoint(const fv2& p, i32 i);
        void SetPointWrap(const fv2& p, i32 i) { SetPoint(p, WrapIndex(i)); }

        const PointWithInvDist& DataAt(i32 i) const { return data[i]; }
        PointWithInvDist& DataAt(i32 i) { return data[i]; }

        const fv2& PointAt(i32 i) const { return DataAt(i).coords; }
        fv2& PointAt(i32 i) { return DataAt(i).coords; }
        float InvLenBtwn(i32 i) const { return DataAt(i).invDist; }
        float& InvLenBtwn(i32 i) { return DataAt(i).invDist; }
    };
} // Quasi
