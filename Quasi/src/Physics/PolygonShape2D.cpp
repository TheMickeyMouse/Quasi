#include "PolygonShape2D.h"

#include <numeric>

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    template <u32 N> u32 PolygonShape<N>::Size() const {
        if constexpr (DYNAMIC) {
            return points.size();
        } else {
            return N;
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
        return std::reduce(points.begin(), points.end(), Math::fVector2 {}) / (float)Size();
    }

    template <u32 N>
    Math::fVector2 PolygonShape<N>::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        u32 nearest = 0;
        float mindist = p.distsq(points[0]);
        for (u32 i = 1; i < Size(); ++i) {
            if (const float d = p.distsq(points[i]); d < mindist) {
                mindist = d;
                nearest = i;
            }
        }
        return xf * points[nearest];
    }

    template <u32 N>
    Math::fVector2 PolygonShape<N>::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const Math::fVector2 invNorm = xf.rotation.inv().rotate(normal);
        u32 furthest = 0;
        float m = invNorm.dot(points[0]);
        for (u32 i = 1; i < Size(); ++i) {
            if (const float d = invNorm.dot(points[i]); d > m) {
                m = d;
                furthest = i;
            }
        }
        return xf * points[furthest];
    }

    template <u32 N>
    Math::fLine2D PolygonShape<N>::BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const Math::fVector2 invNorm = xf.rotation.inv().rotate(normal);
        float maxDepth = invNorm.dot(points[0]);
        u32 furthest = 0;
        for (u32 i = 1; i < Size(); ++i) {
            if (const float d = invNorm.dot(points[i]); d > maxDepth) {
                maxDepth = d;
                furthest = i;
            }
        }

        const Math::fVector2 &f     = points[furthest],
                             &edge0 = PointAt(furthest + 1),
                             &edge1 = PointAt(furthest - 1);
        if (std::abs((f - edge0).norm().dot(invNorm)) > std::abs((f - edge1).norm().dot(invNorm)))
            return { xf * f, xf * edge1 };
        return { xf * f, xf * edge0 };
    }

    template <u32 N>
    Math::fRange PolygonShape<N>::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const Math::fVector2 n = xf.rotation.inv().rotate(axis);
        float min = n.dot(points[0]), max = min;
        for (u32 i = 1; i < Size(); ++i) {
            const float d = n.dot(points[i]);
            min = std::min(min, d);
            max = std::max(max, d);
        }
        const float p = xf.position.dot(axis);
        return { min + p, max + p };
    }

    template <u32 N>
    Math::fRange PolygonShape<N>::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const Math::fVector2 n = xf.rotation.inv().rotate(axis);
        float min = n.dot(points[axisID]), max = min;
        for (u32 i = 0; i < Size(); ++i) {
            if (i == axisID || i == (axisID + 1) % Size()) continue;
            const float d = n.dot(points[i]);
            min = std::min(min, d);
            max = std::max(max, d);
        }
        const float p = xf.position.dot(axis);
        return { min + p, max + p };
    }

    template <u32 N>
    bool PolygonShape<N>::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        bool success = false;
        for (u32 i = 0; i < Size() - 1; ++i) {
            success |= sat.CheckAxis(xf.TransformOffset(points[i + 1] - points[i]).perpend());
        }
        success |= sat.CheckAxis(xf.TransformOffset(points[0] - points[Size() - 1]).perpend());
        return success;
    }

    template class PolygonShape<3>;
    template class PolygonShape<4>;
    template class PolygonShape<0>;
} // Quasi