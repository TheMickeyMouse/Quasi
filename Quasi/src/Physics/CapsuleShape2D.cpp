#include "CapsuleShape2D.h"

#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    float CapsuleShape::ComputeArea() const {
        return radius * (Math::PI * radius + forward.len() * 2);
    }

    Math::fRect2D CapsuleShape::ComputeBoundingBox() const {
        const bool negX = forward.x < 0, negY = forward.y < 0;
        switch (2 * negX + negY) {
            case 0:  return { -radius, forward + radius };
            case 1:  return { { -radius, forward.y - radius }, { forward.x + radius, +radius } };
            case 2:  return { { forward.x - radius, -radius }, { +radius, forward.y + radius } };
            case 3:  return { forward - radius, +radius };
            default: return {};
        }
    }

    Math::fVector2 CapsuleShape::NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const {
        const Math::fVector2 p = xf.TransformInverse(point);
        const float t = std::clamp(p.dot(forward) / forward.lensq(), 0.0f, 1.0f);
        const Math::fVector2 closest = xf * (forward * t);
        return closest + (point - closest).norm(radius);
    }

    Math::fVector2 CapsuleShape::FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const {
        const Math::fVector2& furthest = (0 > xf.TransformOffset(forward).dot(normal) ? xf.position : xf * forward);
        return furthest + normal * radius;
    }

    Math::fRange CapsuleShape::ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const {
        return Math::fRange { xf.position.dot(axis), (xf * forward).dot(axis) }.corrected().extrude(radius);
    }

    Math::fRange CapsuleShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const {
        const float center = xf.position.dot(axis);
        return { center - radius, center + radius };
    }

    bool CapsuleShape::AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const {
        return sat.CheckAxis(xf.TransformOffset(forward).perpend());
    }
} // Quasi