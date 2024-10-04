#include "RectShape2D.h"

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    Math::fVector2 RectShape::Corner(bool px, bool py) const {
        return { px ? hx : -hx, py ? hy : -hy };
    }

    void RectShape::TransformTo(const PhysicsTransform& xf, Out<TransformedVariant*> out) const {
        out->center = xf.position;
        out->x = xf.rotation.as_vec() * hx;
        out->y = xf.rotation.muli().as_vec() * hy;
        out->xInvLen = 1.0f / hx;
        out->yInvLen = 1.0f / hy;
    }

    RectShape::TransformedVariant RectShape::Transform(const PhysicsTransform& xf) const {
        return { *this, xf };
    }

    Math::fVector2 TransformedRectShape::Corner(bool px, bool py) const {
        return center + (px ? x : -x) + (py ? y : -y);
    }

    Math::fRect2D TransformedRectShape::ComputeBoundingBox() const {
        const Math::fVector2 max = {
            std::max(std::abs(x.x + y.x), std::abs(x.x - y.x)),
            std::max(std::abs(x.y + y.y), std::abs(x.y - y.y)) };
        return { center - max, center + max };
    }

    Math::fVector2 TransformedRectShape::NearestPointTo(const Math::fVector2& point) const {
        const Math::fVector2 p = Math::fComplex::from_vec(x).conj().rotate(point - center);
        return Corner(p.x > 0, p.y > 0);
    }

    Math::fVector2 TransformedRectShape::FurthestAlong(const Math::fVector2& normal) const {
        const float dx = x.dot(normal), dy = y.dot(normal);
        return Corner(dx > 0, dy > 0);
    }

    Math::fLine2D TransformedRectShape::BestEdgeFor(const Math::fVector2& normal) const {
        const float dx = x.dot(normal), dy = y.dot(normal);
        const bool xPerpendicular = std::abs(dx) < std::abs(dy);
        return { Corner(dx > 0, dy > 0), Corner(dx > 0 ^ xPerpendicular, dy > 0 ^ !xPerpendicular) };
    }

    Math::fRange TransformedRectShape::ProjectOntoAxis(const Math::fVector2& axis) const {
        const float c = center.dot(axis);
        const float dCorner1 = (x + y).dot(axis), dCorner2 = (x - y).dot(axis),
                    maxD = std::max(std::abs(dCorner1), std::abs(dCorner2));
        return { c - maxD, c + maxD };
    }

    Math::fRange TransformedRectShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const {
        const float c = center.dot(axis);
        const float half = axisID == 0 ? x.dot(axis) : y.dot(axis);
        return { c - half, c + half };
    }

    bool TransformedRectShape::AddSeperatingAxes(SeperatingAxisSolver& sat) const {
        bool success = false;
        success |= sat.CheckAxis(x * xInvLen);
        success |= sat.CheckAxis(y * yInvLen);
        return success;
    }
} // Quasi