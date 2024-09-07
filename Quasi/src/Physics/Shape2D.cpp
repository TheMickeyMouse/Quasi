#include "Shape2D.h"

#include <algorithm>

#include "Collision2D.h"
#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    Math::fLine2D Shape::BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        return { {}, {} };
    }

    Manifold Shape::CollideWith(const PhysicsTransform& xf, const Shape& other, const PhysicsTransform& xfOther) const {
        return Collision::CollideShapes(*this, xf, other, xfOther);
    }

    // circ

    float CircleShape::ComputeArea() const {
        return Math::PI * radius * radius;
    }

    Math::fRect2D CircleShape::ComputeBoundingBox() const {
        return { -radius, radius };
    }

    Math::fVector2 CircleShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        return xf.position + (point - xf.position).norm(radius);
    }

    Math::fVector2 CircleShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        return xf.position + normal * radius;
    }

    Math::fRange CircleShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = axis.dot(xf.position);
        return { center - radius, center + radius };
    }

    Math::fRange CircleShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        return ProjectOntoAxis(axis, xf);
    }

    // edge

    float EdgeShape::ComputeArea() const {
        return Math::PI * radius * radius + start.dist(end) * radius * 2;
    }

    Math::fRect2D EdgeShape::ComputeBoundingBox() const {
        return Math::fRect2D(start, end).corrected().extrude(radius);
    }

    Math::fVector2 EdgeShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        const float t = std::clamp((p - start).dot(end - start) / (end - start).lensq(), 0.0f, 1.0f);
        const Math::fVector2 closest = xf * (start + (end - start) * t);
        return closest + (point - closest).norm(radius);
    }

    Math::fVector2 EdgeShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const Math::fVector2 s = xf * start, e = xf * end, &furthest = (s.dot(normal) > e.dot(normal) ? s : e);
        return furthest + normal * radius;
    }

    Math::fRange EdgeShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        return Math::fRange { (xf * start).dot(axis), (xf * end).dot(axis) }.corrected().extrude(radius);
    }

    Math::fRange EdgeShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = (xf * start).dot(axis);
        return { center - radius, center + radius };
    }

    bool EdgeShape::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        return sat.CheckAxis(xf.TransformOffset(end - start).perpend());
    }

    // tri

    float TriangleShape::ComputeArea() const {
        return std::abs(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) * 0.5f;
    }

    Math::fRect2D TriangleShape::ComputeBoundingBox() const {
        const Span<const Math::fVector2> span = { &a, 3 };
        return { Math::fVector2::min(span), Math::fVector2::max(span) };
    }

    Math::fVector2 TriangleShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        const float dist[] = { p.distsq(a), p.distsq(b), p.distsq(c), };
        switch (std::min_element(dist, dist + 3) - dist) {
            case 0: return xf * a;
            case 1: return xf * b;
            case 2: return xf * c;
            default: return {};
        }
    }

    Math::fVector2 TriangleShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        constexpr Math::fVector2 TriangleShape::* points[] = { &TriangleShape::b, &TriangleShape::c };
        float maxDepth = (xf * a).dot(normal);
        const Math::fVector2* furthest = &a;
        for (const auto point : points) {
            if (const float d = (xf * this->*point).dot(normal); d > maxDepth) {
                maxDepth = d;
                furthest = &(this->*point);
            }
        }
        return *furthest;
    }

    Math::fLine2D TriangleShape::BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const Math::fVector2 points[] = { xf * a, xf * b, xf * c };
        float maxDepth = points[0].dot(normal);
        u32 furthest = 0;
        for (u32 i = 1; i < 3; ++i) {
            if (const float d = points[i].dot(normal); d > maxDepth) {
                maxDepth = d;
                furthest = i;
            }
        }

        const Math::fVector2 &f     = points[furthest],
                             &edge0 = points[0x102 >> (furthest * 4) & 0xF],
                             &edge1 = points[0x021 >> (furthest * 4) & 0xF];
        if (std::abs((f - edge0).norm().dot(normal)) > std::abs((f - edge1).norm().dot(normal)))
            return { f, edge1 };
        return { f, edge0 };
    }

    Math::fRange TriangleShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float coordinates[3] = { (xf * a).dot(axis), (xf * b).dot(axis), (xf * c).dot(axis) };
        const auto [min, max] = std::minmax_element(coordinates, coordinates + 3);
        return { *min, *max };
    }

    Math::fRange TriangleShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        constexpr Math::fVector2 TriangleShape::* points[] = {
            &TriangleShape::c, &TriangleShape::a, &TriangleShape::b, &TriangleShape::c
        };
        return Math::fRange { (xf * this->*points[axisID + 1]).dot(axis), (xf * this->*points[axisID]).dot(axis) }.corrected();
    }

    bool TriangleShape::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        bool success = false;
        success |= sat.CheckAxis(xf.TransformOffset(b - a).perpend());
        success |= sat.CheckAxis(xf.TransformOffset(c - b).perpend());
        success |= sat.CheckAxis(xf.TransformOffset(a - c).perpend());
        return success;
    }

    Math::fVector2 RectShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        const u32 corner = (2 * p.y > rect.min.y + rect.max.y) << 1 | (2 * p.x > rect.min.x + rect.max.x);
        return xf * rect.corner(corner);
    }

    Math::fVector2 RectShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const float x = xf.rotation.as_vec().dot(normal), y = xf.rotation.muli().as_vec().dot(normal);
        return xf * rect.corner(((y > 0) << 1) | (x > 0));
    }

    Math::fLine2D RectShape::BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const float x = xf.rotation.as_vec().dot(normal), y = xf.rotation.muli().as_vec().dot(normal);
        const bool xPerpendicular = std::abs(x * rect.width()) < std::abs(y * rect.height());
        const u32 corner = ((y > 0) << 1) | (x > 0);
        return { xf * rect.corner(corner), xf * rect.corner(corner ^ (xPerpendicular ? 1 : 2)) };
    }

    Math::fRange RectShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = (xf * rect.center()).dot(axis);
        const float dCorner1 = (xf * rect.min).dot(axis), dCorner2 = (xf * rect.corner(1)).dot(axis),
                    maxD = std::max(std::abs(dCorner1 - center), std::abs(dCorner2 - center));
        return { center - maxD, center + maxD };
    }

    Math::fRange RectShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = (xf * rect.center()).dot(axis);
        const float half = (axisID & 1 ? rect.height() : rect.width()) * 0.5f;
        return { center - half, center + half };
    }

    bool RectShape::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        bool success = false;
        success |= sat.CheckAxis(xf.rotation.as_vec());
        success |= sat.CheckAxis(xf.rotation.muli().as_vec());
        return success;
    }
    // rect

}
