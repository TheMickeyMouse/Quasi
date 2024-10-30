#include "CircleShape2D.h"

namespace Quasi::Physics2D {
    float CircleShape::ComputeArea() const {
        return PI * radius * radius;
    }

    fRect2D CircleShape::ComputeBoundingBox() const {
        return { -radius, radius };
    }

    float CircleShape::Inertia() const {
        return 0.5f * radius * radius;
    }

    fVector2 CircleShape::NearestPointTo(const fVector2& point) const {
        return point.norm(radius);
    }

    fVector2 CircleShape::FurthestAlong(const fVector2& normal) const {
        return normal * radius;
    }

    fRange CircleShape::ProjectOntoAxis(const fVector2& axis) const {
        return { -radius, +radius };
    }

    fRange CircleShape::ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const {
        return ProjectOntoAxis(axis);
    }
} // Quasi