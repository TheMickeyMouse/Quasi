#include "RectShape2D.h"

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    Math::fVector2 RectShape::Corner(bool px, bool py) const {
        return { px ? hx : -hx, py ? hy : -hy };
    }

    Math::fVector2 RectShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        return xf * Corner(p.x > 0, p.y > 0);
    }

    Math::fVector2 RectShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const float x = xf.rotation.as_vec().dot(normal), y = xf.rotation.muli().as_vec().dot(normal);
        return xf * Corner(x > 0, y > 0);
    }

    Math::fLine2D RectShape::BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const float x = xf.rotation.as_vec().dot(normal), y = xf.rotation.muli().as_vec().dot(normal);
        const bool xPerpendicular = std::abs(x * hx) < std::abs(y * hy);
        return { xf * Corner(x > 0, y > 0), xf * Corner(x > 0 ^ xPerpendicular, y > 0 ^ !xPerpendicular) };
    }

    Math::fRange RectShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = xf.position.dot(axis);
        const float dCorner1 = (xf * Corner(false, false)).dot(axis), dCorner2 = (xf * Corner(true, false)).dot(axis),
                    maxD = std::max(std::abs(dCorner1 - center), std::abs(dCorner2 - center));
        return { center - maxD, center + maxD };
    }

    Math::fRange RectShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = xf.position.dot(axis);
        const float half = axisID & 1 ? hy : hx;
        return { center - half, center + half };
    }

    bool RectShape::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        bool success = false;
        success |= sat.CheckAxis(xf.rotation.as_vec());
        success |= sat.CheckAxis(xf.rotation.muli().as_vec());
        return success;
    }
} // Quasi