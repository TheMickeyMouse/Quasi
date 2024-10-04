#include "PolygonShape2D.h"

#include <numeric>

#include "Logger.h"
#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    template <u32 N> PolygonShape<N>::PolygonShape(decltype(points)&& ps) {
        i32 clockRotation = 0; // positive means clockwise
        for (u32 i = 1; i <= ps.size(); ++i) {
            const auto& prev = ps[i - 1], curr = ps[i], next = ps[(i + 1) % ps.size()];
            const float dx1 = prev.x - curr.x, dy1 = prev.y - curr.y,
                        dx2 = next.x - curr.x, dy2 = next.y - curr.y,
                        zcross = dx1 * dy2 - dx2 * dy1;
            const i32 sign = (zcross > 0) - (zcross < 0);
            Debug::Assert(sign * clockRotation == -1, "Polygon is not convex: {}", ps);
            clockRotation |= sign;
        }

        if (clockRotation > 0) {
            if constexpr (DYNAMIC) points.resize(ps.size());
            std::reverse_copy(ps.begin(), ps.end(), points.begin());  // reverse
        } else {
            points = std::move(ps);
        }
    }

    template <u32 N> i32 PolygonShape<N>::Size() const {
        if constexpr (DYNAMIC) {
            return (i32)points.size();
        } else {
            return (i32)N;
        }
    }

    template <u32 N> void PolygonShape<N>::AddPoint(const Math::fVector2& p) requires DYNAMIC {
        points.emplace_back(p);
    }

    template <u32 N> void PolygonShape<N>::AddPoint(const Math::fVector2& p, u32 i) requires DYNAMIC {
        points.insert(points.begin() + i, p);
    }

    template <u32 N> void PolygonShape<N>::RemovePoint(u32 i) requires DYNAMIC {
        points.erase(points.begin() + i);
    }

    template <u32 N> void PolygonShape<N>::PopPoint() requires DYNAMIC {
        points.pop_back();
    }

    template <u32 N>
    float PolygonShape<N>::ComputeArea() const {
        float area = 0;
        for (u32 i = 0; i < Size(); i += 2)
            area += Xof(i+1) * (Yof(i+2) - Yof(i)) + Yof(i+1) * (Xof(i) - Xof(i+2));
        area /= 2;
        return area;
    }

    template <u32 N>
    Math::fRect2D PolygonShape<N>::ComputeBoundingBox() const {
        return Math::fRect2D::over(points);
    }

    template <u32 N>
    Math::fVector2 PolygonShape<N>::CenterOfMass() const {
        float x = 0.0f, y = 0.0f, area = 0.0f;
        for(int i = 0; i < Size() - 1; i++) {
            const Math::fVector2& p0 = points[i], &p1 = points[i+1];
            const float areaSum = (p0.x * p1.y) - (p1.x * p0.y);

            x += (p0.x + p1.x) * areaSum;
            y += (p0.y + p1.y) * areaSum;
            area += areaSum;
        }
        const float invArea = 1.0f / 6.0f / area;
        return { x * invArea, y * invArea };
    }

    template <u32 N> void PolygonShape<N>::TransformTo(const PhysicsTransform& xf, Out<TransformedVariant*> out) const {
        if constexpr (DYNAMIC)
            out->points.resize(Size());
        std::transform(points.begin(), points.end(), out->points.begin(),
                [&] (const Math::fVector2& p) { return xf.Transform(p); });
    }

    template <u32 N>
    PolygonShape<N> PolygonShape<N>::Transform(const PhysicsTransform& xf) const {
        return { *this, xf };
    }

    template <u32 N>
    Math::fVector2 PolygonShape<N>::NearestPointTo(const Math::fVector2& point) const {
        u32 nearest = 0;
        float mindist = point.distsq(points[0]);
        for (u32 i = 1; i < Size(); ++i) {
            if (const float d = point.distsq(points[i]); d < mindist) {
                mindist = d;
                nearest = i;
            }
        }
        return points[nearest];
    }

    template <u32 N>
    Math::fVector2 PolygonShape<N>::FurthestAlong(const Math::fVector2& normal) const {
        u32 furthest = 0;
        float m = normal.dot(points[0]);
        for (u32 i = 1; i < Size(); ++i) {
            if (const float d = normal.dot(points[i]); d > m) {
                m = d;
                furthest = i;
            }
        }
        return points[furthest];
    }

    template <u32 N>
    Math::fLine2D PolygonShape<N>::BestEdgeFor(const Math::fVector2& normal) const {
        float maxDepth = normal.dot(points[0]);
        i32 furthest = 0;
        for (i32 i = 1; i < Size(); ++i) {
            if (const float d = normal.dot(points[i]); d > maxDepth) {
                maxDepth = d;
                furthest = i;
            }
        }

        const Math::fVector2 &f     = points[furthest],
                             &edge0 = PointAt(furthest + 1),
                             &edge1 = PointAt(furthest - 1);
        if (std::abs((f - edge0).norm().dot(normal)) > std::abs((f - edge1).norm().dot(normal)))
            return { f, edge1 };
        return { f, edge0 };
    }

    template <u32 N>
    Math::fRange PolygonShape<N>::ProjectOntoAxis(const Math::fVector2& axis) const {
        Math::fRange range = Math::fRange::unrange();
        for (u32 i = 0; i < Size(); ++i) {
            const float d = axis.dot(points[i]);
            range = range.expand_until(d);
        }
        return range;
    }

    template <u32 N>
    Math::fRange PolygonShape<N>::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const {
        Math::fRange range = Math::fRange::at(axis.dot(points[axisID]));
        for (u32 i = 0; i < Size(); ++i) {
            if (i == axisID || i == (axisID + 1) % Size()) continue;
            const float d = axis.dot(points[i]);
            range = range.expand_until(d);
        }
        return range;
    }

    template <u32 N>
    bool PolygonShape<N>::AddSeperatingAxes(SeperatingAxisSolver& sat) const {
        bool success = false;
        for (u32 i = 0; i < Size(); ++i) {
            success |= sat.CheckAxis((PointAt(i + 1) - points[i]).perpend().norm());
        }
        return success;
    }

    template class PolygonShape<3>;
    template class PolygonShape<4>;
    template class PolygonShape<0>;
} // Quasi