#pragma once
#include "CapsuleShape2D.h"
#include "CircleShape2D.h"
#include "IShape2D.h"
#include "Logger.h"
#include "Match.h"
#include "PolygonShape2D.h"
#include "RectShape2D.h"
#include "Variant.h"

namespace Quasi::Physics2D {
    class Shape : public IShape, public Variant<CircleShape, CapsuleShape, RectShape, TriangleShape, QuadShape, DynPolyShape> {
    public:
        enum Type { CIRCLE, CAPSULE, RECT, TRI, QUAD, POLY };

        Shape() : Variant(CircleShape { 0.0f }) {}
        Shape(Variant v) : Variant(std::move(v)) {}
        Shape(const std::derived_from<IShape> auto& s) : Variant(s) {}
        Shape(std::derived_from<IShape> auto&& s) : Variant(std::forward<decltype(s)>(s)) {}

        template <class T> static constexpr Type TypeIndexOf();
        [[nodiscard]] Type TypeIndex() const;

        static ClipPrimitive PrimitiveOf(Type t);
        [[nodiscard]] ClipPrimitive PreferedPrimitive() const;

        [[nodiscard]] float ComputeArea() const;
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;
        [[nodiscard]] Math::fVector2 CenterOfMass() const;

        [[nodiscard]] Math::fVector2 NearestPointTo(const Math::fVector2& point, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fVector2 FurthestAlong(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fLine2D BestEdgeFor(const Math::fVector2& normal, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoAxis(const Math::fVector2& axis, const PhysicsTransform& xf) const;
        [[nodiscard]] Math::fRange ProjectOntoOwnAxis(u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf) const;
        bool AddSeperatingAxes(SeperatingAxisSolver& sat, const PhysicsTransform& xf) const;
    };

    template <class T> constexpr Shape::Type Shape::TypeIndexOf() {
        qmatch ((typename)T, (
            case (CircleShape)   { return CIRCLE; },
            case (CapsuleShape)  { return CAPSULE; },
            case (RectShape)     { return RECT; },
            case (TriangleShape) { return TRI; },
            case (QuadShape)     { return QUAD; },
            case (DynPolyShape)  { return POLY; },
            else { static_assert(std::is_same_v<T, std::monostate>, "oops"); return CIRCLE; }
        ));
    }
} // Quasi
