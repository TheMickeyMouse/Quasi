#include "Shape2D.h"

namespace Quasi::Physics2D {
    Shape::Type Shape::TypeIndex() const {
        return (Type)ID();
    }

    IShape::ClipPrimitive Shape::PrimitiveOf(Type t) {
        return (ClipPrimitive)((t >= CAPSULE) + (t >= RECT));
    }

    IShape::ClipPrimitive Shape::PreferedPrimitive() const {
        return PrimitiveOf(TypeIndex());
    }

#define IMPLEMENT_SHAPE_FN(N, PARAMS, ARGS) Shape::N PARAMS const { return Visit([&] (const auto& s) { return s.N ARGS; }); }

    float          IMPLEMENT_SHAPE_FN(ComputeArea,        (), ())
    Math::fRect2D  IMPLEMENT_SHAPE_FN(ComputeBoundingBox, (), ())
    Math::fVector2 IMPLEMENT_SHAPE_FN(CenterOfMass,       (), ())

    Math::fVector2 IMPLEMENT_SHAPE_FN(NearestPointTo,     (const Math::fVector2& point, const PhysicsTransform& xf),            (point, xf))
    Math::fVector2 IMPLEMENT_SHAPE_FN(FurthestAlong,      (const Math::fVector2& normal, const PhysicsTransform& xf),           (normal, xf))
    Math::fLine2D  IMPLEMENT_SHAPE_FN(BestEdgeFor,        (const Math::fVector2& normal, const PhysicsTransform& xf),           (normal, xf))
    Math::fRange   IMPLEMENT_SHAPE_FN(ProjectOntoAxis,    (const Math::fVector2& axis, const PhysicsTransform& xf),             (axis, xf))
    Math::fRange   IMPLEMENT_SHAPE_FN(ProjectOntoOwnAxis, (u32 axisID, const Math::fVector2& axis, const PhysicsTransform& xf), (axisID, axis, xf))
    bool           IMPLEMENT_SHAPE_FN(AddSeperatingAxes,  (SeperatingAxisSolver& sat, const PhysicsTransform& xf),              (sat, xf))
} // Quasi