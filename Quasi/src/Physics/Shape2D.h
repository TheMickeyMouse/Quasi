#pragma once
#include "Geometry.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"

namespace Quasi::Physics2D {
    enum ShapeType { CIRCLE, EDGE, TRIANGLE, MAX };

    class Shape {
    public:
        virtual ~Shape() = default;
        [[nodiscard]] virtual Ref<Shape> CloneOn(ArenaAllocator&) const = 0;
        [[nodiscard]] virtual float ComputeArea() const = 0;
        [[nodiscard]] virtual Math::fRect2D ComputeBoundingBox() const = 0;
        [[nodiscard]] virtual ShapeType TypeIndex() const = 0;
    };

    template <class S, ShapeType T> class SmartShape : public Shape {
        [[nodiscard]] Ref<Shape> CloneOn(ArenaAllocator&) const override;
        [[nodiscard]] ShapeType TypeIndex() const override { return T; }
        using Shape::Shape;
    };

    class CircleShape : public SmartShape<CircleShape, CIRCLE> {
    public:
        float radius = 1.0f;

        CircleShape(float r) : radius(r) {}
        ~CircleShape() override = default;
        [[nodiscard]] float ComputeArea() const override;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override;
    };

    class EdgeShape : public SmartShape<EdgeShape, EDGE> {
    public:
        Math::fVector2 start, end;
        float radius = 0.0f;

        EdgeShape(const Math::fVector2& s, const Math::fVector2& e, float r) : start(s), end(e), radius(r) {}
        ~EdgeShape() override = default;
        [[nodiscard]] float ComputeArea() const override;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override;
    };

    class TriangleShape : public SmartShape<TriangleShape, TRIANGLE> {
    public:
        Math::fVector2 a, b, c;

        TriangleShape(const Math::fVector2& a, const Math::fVector2& b, const Math::fVector2& c) : a(a), b(b), c(c) {}
        ~TriangleShape() override = default;

        [[nodiscard]] float ComputeArea() const override;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override;
    };

    template <class S, ShapeType T>
    Ref<Shape> SmartShape<S, T>::CloneOn(ArenaAllocator& allocator) const {
        return DerefPtr(allocator.Create<S>(*static_cast<const S*>(this)));
    }
}
