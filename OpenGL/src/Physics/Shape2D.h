#pragma once
#include "Geometry.h"
#include "Rect.h"

namespace Physics2D {
    class Shape {
    public:
        virtual ~Shape() = default;
        [[nodiscard]] virtual Shape* CloneHeap() const = 0;
        [[nodiscard]] virtual float ComputeArea() const = 0;
        [[nodiscard]] virtual Maths::rect2f ComputeBoundingBox() const = 0;
    };

    template <class S> class CloneableShape : public Shape {
        [[nodiscard]] Shape* CloneHeap() const override;
        using Shape::Shape;
    };

    class CircleShape : public CloneableShape<CircleShape> {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        ~CircleShape() override = default;
        [[nodiscard]] float ComputeArea() const override { return Maths::PI * radius * radius; }
        [[nodiscard]] Maths::rect2f ComputeBoundingBox() const override { return { -radius, radius }; }
    };

    class EdgeShape : public CloneableShape<EdgeShape> {
    public:
        Maths::fvec2 start, end;

        EdgeShape(const Maths::fvec2& s, const Maths::fvec2& e) : start(s), end(e) {}
        ~EdgeShape() override = default;
        [[nodiscard]] float ComputeArea() const override { return 0; }
        [[nodiscard]] Maths::rect2f ComputeBoundingBox() const override { return Maths::rect2f(start, end).corrected(); }
    };

    template <class S>
    Shape* CloneableShape<S>::CloneHeap() const {
        return new S(*static_cast<const S*>(this));
    }
}
