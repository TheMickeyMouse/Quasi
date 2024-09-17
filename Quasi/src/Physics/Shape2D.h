#pragma once
#include "CapsuleShape2D.h"
#include "CircleShape2D.h"
#include "Match.h"
#include "PolygonShape2D.h"
#include "RectShape2D.h"
#include "Variant.h"


namespace Quasi::Physics2D {
    template <class T> static constexpr IShape::Type ShapeTypeIndexOf() {
        qmatch ((typename)T, (
            case (CircleShape)   { return IShape::CIRCLE; },
            case (CapsuleShape)  { return IShape::CAPSULE; },
            case (RectShape)     { return IShape::RECT; },
            case (TriangleShape) { return IShape::TRI; },
            case (QuadShape)     { return IShape::QUAD; },
            case (DynPolyShape)  { return IShape::POLY; },
            else { static_assert(std::is_same_v<T, std::monostate>, "oops"); return IShape::CIRCLE; }
        ));
    }
    static constexpr IShape::ClipPrimitive PrimitiveOfType(IShape::Type t) {
        return (IShape::ClipPrimitive)((t >= IShape::CAPSULE) + (t >= IShape::RECT));
    }

    class TransformedShape;

    template <class T>
    class VariantShape {
    public:
        [[nodiscard]] IShape::Type TypeIndex() const { return (IShape::Type)((const T*)this)->ID(); }
        [[nodiscard]] IShape::ClipPrimitive PreferedPrimitive() const { return PrimitiveOfType(TypeIndex()); }
    };

    class Shape :
        public IShape,
        public VariantShape<Shape>,
        public Variant<CircleShape, CapsuleShape, RectShape, TriangleShape, QuadShape, DynPolyShape> {
    public:
        Shape() : Variant(CircleShape { 0.0f }) {}
        Shape(Variant v) : Variant(std::move(v)) {}
        Shape(const std::derived_from<IShape> auto& s) : Variant(s) {}
        Shape(std::derived_from<IShape> auto&& s) : Variant(std::forward<decltype(s)>(s)) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const;

        [[nodiscard]] TransformedShape Transform(const PhysicsTransform& xf) const;
    };

    class TransformedShape :
        public ITransformedShape,
        public VariantShape<TransformedShape>,
        public Variant<TransformedCircleShape, TransformedCapsuleShape, TransformedRectShape, TriangleShape, QuadShape, DynPolyShape> {
    public:
        TransformedShape() : Variant(TransformedCircleShape { 0, 0.0f }) {}
        TransformedShape(Variant v) : Variant(std::move(v)) {}
        TransformedShape(const std::derived_from<ITransformedShape> auto& s) : Variant(s) {}
        TransformedShape(std::derived_from<ITransformedShape> auto&& s) : Variant(std::forward<decltype(s)>(s)) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const;

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;
    };
} // Quasi
