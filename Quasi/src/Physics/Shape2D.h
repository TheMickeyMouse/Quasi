#pragma once
#include "Geometry.h"
#include "PhysicsTransform2D.h"
#include "Rect.h"
#include "Utils/ArenaAllocator.h"

namespace Quasi::Physics2D {
    class SeperatingAxisSolver;
    struct Manifold;

    enum ShapeType { CIRCLE, EDGE, TRIANGLE, RECT, MAX };

    class Shape {
    public:
        virtual ~Shape() = default;
        [[nodiscard]] virtual Ref<Shape> CloneOn(ArenaAllocator&) const = 0;
        [[nodiscard]] virtual float ComputeArea() const = 0;
        [[nodiscard]] virtual Math::fRect2D ComputeBoundingBox() const = 0;
        [[nodiscard]] virtual Math::fVector2 CenterOfMass() const = 0;
        [[nodiscard]] virtual ShapeType TypeIndex() const = 0;

        // SAT helpers
        enum ClipPrimitive { CIRCLE, POINT, EDGE };
        [[nodiscard]] virtual Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const = 0;
        [[nodiscard]] virtual Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const = 0;
        [[nodiscard]] virtual ClipPrimitive PreferedClipPrimitive() const = 0;
        [[nodiscard]] virtual Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] virtual Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const = 0;
        [[nodiscard]] virtual Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const = 0;
        virtual bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const = 0;


        [[nodiscard]] Manifold CollideWith(const PhysicsTransform& xf, const Shape& other, const PhysicsTransform& xfOther) const;
    };

    template <class S, ShapeType T> class SmartShape : public Shape {
    public:
        [[nodiscard]] Ref<Shape> CloneOn(ArenaAllocator&) const override;
        static constexpr ShapeType Index = T;
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
        [[nodiscard]] Math::fVector2 CenterOfMass() const override { return 0; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] ClipPrimitive PreferedClipPrimitive() const override { return CIRCLE; }
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const override { return false; }
    };

    class EdgeShape : public SmartShape<EdgeShape, EDGE> {
    public:
        Math::fVector2 start, end;
        float radius = 0.0f;

        EdgeShape(const Math::fVector2& s, const Math::fVector2& e, float r) : start(s), end(e), radius(r) {}
        ~EdgeShape() override = default;
        [[nodiscard]] float ComputeArea() const override;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override;
        [[nodiscard]] Math::fVector2 CenterOfMass() const override { return (start + end) * 0.5f; }
        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] ClipPrimitive PreferedClipPrimitive() const override { return POINT; }
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const override;
    };

    class TriangleShape : public SmartShape<TriangleShape, TRIANGLE> {
    public:
        Math::fVector2 a, b, c;

        TriangleShape(const Math::fVector2& a, const Math::fVector2& b, const Math::fVector2& c) : a(a), b(b), c(c) {}
        ~TriangleShape() override = default;

        [[nodiscard]] float ComputeArea() const override;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override;
        [[nodiscard]] Math::fVector2 CenterOfMass() const override { return (a + b + c) / 3.0f; }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] ClipPrimitive PreferedClipPrimitive() const override { return EDGE; }
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const override;
    };

    class RectShape : public SmartShape<RectShape, RECT> {
    public:
        Math::fRect2D rect;

        RectShape(const Math::fRect2D& rect) : rect(rect) {}
        ~RectShape() override = default;

        [[nodiscard]] float ComputeArea() const override { return rect.area(); }
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const override { return rect; }
        [[nodiscard]] Math::fVector2 CenterOfMass() const override { return rect.center(); }

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] ClipPrimitive PreferedClipPrimitive() const override { return EDGE; }
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const override;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const override;
    };

    template <class S, ShapeType T>
    Ref<Shape> SmartShape<S, T>::CloneOn(ArenaAllocator& allocator) const {
        return DerefPtr(allocator.Create<S>(*static_cast<const S*>(this)));
    }

    template <class T> concept IShape = std::is_base_of_v<Shape, T>;
}
