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
        Shape(const Extends<IShape> auto& s) : Variant(s) {}
        Shape(Extends<IShape> auto&& s) : Variant(std::forward<decltype(s)>(s)) {}

        float ComputeArea() const;
        fRect2D ComputeBoundingBox() const;
        float Inertia() const;

        fVector2 NearestPointTo(const fVector2& point) const;
        fVector2 FurthestAlong(const fVector2& normal) const;
        fLine2D BestEdgeFor(const fVector2& normal) const;
        fRange ProjectOntoAxis(const fVector2& axis) const;
        fRange ProjectOntoOwnAxis(u32 axisID, const fVector2& axis) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat) const;

        Type TypeIndex() const { return (Type)ID(); }
        ClipPrimitive PreferedPrimitive() const { return PrimitiveOfType(TypeIndex()); }
    };

    Shape MakePolygon(Span<const fVector2> points);
} // Quasi
