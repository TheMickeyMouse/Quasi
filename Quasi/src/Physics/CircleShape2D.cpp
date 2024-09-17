#include "CircleShape2D.h"

namespace Quasi::Physics2D {
    float CircleShape::ComputeArea() const {
        return Math::PI * radius * radius;
    }

    Math::fRect2D CircleShape::ComputeBoundingBox() const {
        return { -radius, radius };
    }

    TransformedCircleShape CircleShape::Transform(const PhysicsTransform& xf) const {
        return { xf.position, radius };
    }

    float TransformedCircleShape::ComputeArea() const {
        return Math::PI * radius * radius;
    }

    Math::fRect2D TransformedCircleShape::ComputeBoundingBox() const {
        return { center - radius, center + radius };
    }

    Math::fVector2 TransformedCircleShape::NearestPointTo(const Math::fVector2& point) const {
        return center + (point - center).norm(radius);
    }

    Math::fVector2 TransformedCircleShape::FurthestAlong(const Math::fVector2& normal) const {
        return center + normal * radius;
    }

    Math::fRange TransformedCircleShape::ProjectOntoAxis(const Math::fVector2& axis) const {
        const float c = axis.dot(center);
        return { c - radius, c + radius };
    }

    Math::fRange TransformedCircleShape::ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const {
        return ProjectOntoAxis(axis);
    }
} // Quasi