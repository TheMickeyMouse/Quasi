#pragma once
#include "CapsuleShape2D.h"
#include "CircleShape2D.h"
#include "Match.h"
#include "PolygonShape2D.h"
#include "RectShape2D.h"
#include "Variant.h"

namespace Quasi::Physics2D {
    template <class T> static constexpr IShape::Type ShapeTypeIndexOf() {
        Qmatch$ ((typename)T, (
            case (CircleShape)     { return IShape::CIRCLE; },
            case (CapsuleShape)    { return IShape::CAPSULE; },
            case (RectShape)       { return IShape::RECT; },
            case (TriangleShape)   { return IShape::TRI; },
            case (QuadShape)       { return IShape::QUAD; },
            case (DynPolygonShape) { return IShape::POLY; },
            else { static_assert(std::is_same_v<T, std::monostate>, "oops"); return IShape::CIRCLE; }
        ));
    }
    static constexpr IShape::ClipPrimitive PrimitiveOfType(IShape::Type t) {
        return (IShape::ClipPrimitive)((t >= IShape::CAPSULE) + (t >= IShape::RECT));
    }

    class Shape :
        public IShape,
        public Variant<CircleShape, CapsuleShape, RectShape, TriangleShape, QuadShape, DynPolygonShape> {
    public:
        Shape() : Variant(CircleShape { 0.0f }) {}
        Shape(Variant v) : Variant(std::move(v)) {}
        Shape(const std::derived_from<IShape> auto& s) : Variant(s) {}
        Shape(std::derived_from<IShape> auto&& s) : Variant(std::forward<decltype(s)>(s)) {}

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] fRect2D ComputeBoundingBox() const;
        [[nodiscard]] fVector2 CenterOfMass() const;
        [[nodiscard]] float Inertia() const;

        [[nodiscard]] fVector2 NearestPointTo(const fVector2& point) const;
        [[nodiscard]] fVector2 FurthestAlong(const fVector2& normal) const;
        [[nodiscard]] fLine2D BestEdgeFor(const fVector2& normal) const;
        [[nodiscard]] fRange ProjectOntoAxis(const fVector2& axis) const;
        [[nodiscard]] fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;

        [[nodiscard]] Type TypeIndex() const { return (Type)ID(); }
        [[nodiscard]] ClipPrimitive PreferedPrimitive() const { return PrimitiveOfType(TypeIndex()); }
    };
} // Quasi
