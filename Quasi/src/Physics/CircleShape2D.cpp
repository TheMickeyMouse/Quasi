#include "CircleShape2D.h"

namespace Quasi::Physics2D {
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
} // Quasi