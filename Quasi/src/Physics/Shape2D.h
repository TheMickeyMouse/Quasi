#pragma once
#include "Geometry.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"

namespace Quasi::Physics2D {
    class Shape {
    public:
        virtual ~Shape() = default;
        [[nodiscard]] virtual Ref<Shape> CloneOn(ArenaAllocator&) const = 0;
        [[nodiscard]] virtual float ComputeArea() const = 0;
        [[nodiscard]] virtual Math::fRect2D ComputeBoundingBox() const = 0;
    };

    template <class S> class CloneableShape : public Shape {
        [[nodiscard]] Ref<Shape> CloneOn(ArenaAllocator&) const override;
        using Shape::Shape;
    };

    class CircleShape : public CloneableShape<CircleShape> {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        ~CircleShape() override = default;
        [[nodiscard]] float ComputeArea() const override { return Math::PI * radius * radius; }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override { return { -radius, radius }; }
    };

    class EdgeShape : public CloneableShape<EdgeShape> {
    public:
        Math::fVector2 start, end;
        float radius = 0.0f;

        EdgeShape(const Math::fVector2& s, const Math::fVector2& e, float r) : start(s), end(e), radius(r) {}
        ~EdgeShape() override = default;
        [[nodiscard]] float ComputeArea() const override { return Math::PI * radius * radius + start.dist(end) * radius * 2; }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override { return Math::fRect2D(start, end).corrected().extrude(radius); }
    };

    template <class S>
    Ref<Shape> CloneableShape<S>::CloneOn(ArenaAllocator& allocator) const {
        return DerefPtr(allocator.Create<S>(*static_cast<const S*>(this)));
    }
}
