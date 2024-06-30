#include "Shape2D.h"

namespace Quasi::Physics2D {
    // circ

    float CircleShape::ComputeArea() const {
        return Math::PI * radius * radius;
    }

    Math::fRect2D CircleShape::ComputeBoundingBox() const {
        return { -radius, radius };
    }

    // edge

    float EdgeShape::ComputeArea() const {
        return Math::PI * radius * radius + start.dist(end) * radius * 2;
    }

    Math::fRect2D EdgeShape::ComputeBoundingBox() const {
        return Math::fRect2D(start, end).corrected().extrude(radius);
    }

    // tri

    float TriangleShape::ComputeArea() const {
        return std::abs(a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) * 0.5f;
    }

    Math::fRect2D TriangleShape::ComputeBoundingBox() const {
        const Span<const Math::fVector2> span = { &a, 3 };
        return { Math::fVector2::min(span), Math::fVector2::max(span) };
    }
}
