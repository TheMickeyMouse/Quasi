#include "Shape2D.h"

namespace Quasi::Physics2D {
#define IMPLEMENT_SHAPE_FN(K, N, PARAMS, ARGS) K::N PARAMS const { return Visit([&] (const auto& s) { return s.N ARGS; }); }

    float            IMPLEMENT_SHAPE_FN(Shape, ComputeArea,        (), ())
    Math::fRect2D    IMPLEMENT_SHAPE_FN(Shape, ComputeBoundingBox, (), ())
    Math::fVector2   IMPLEMENT_SHAPE_FN(Shape, CenterOfMass,       (), ())
    void Shape::TransformTo(const PhysicsTransform& xf, Out<TransformedVariant*> out) const {
        Visit([&]<class T>(const T& s) {
            using R = typename T::TransformedVariant;
            if (!out->Is<R>()) out->Set(R {});
            s.TransformTo(xf, out->As<R>().Address());
        });
    }

    float          IMPLEMENT_SHAPE_FN(TransformedShape, ComputeArea,        (), ())
    Math::fRect2D  IMPLEMENT_SHAPE_FN(TransformedShape, ComputeBoundingBox, (), ())
    Math::fVector2 IMPLEMENT_SHAPE_FN(TransformedShape, CenterOfMass,       (), ())

    Math::fVector2 IMPLEMENT_SHAPE_FN(TransformedShape, NearestPointTo,     (const Math::fVector2& point),            (point))
    Math::fVector2 IMPLEMENT_SHAPE_FN(TransformedShape, FurthestAlong,      (const Math::fVector2& normal),           (normal))
    Math::fLine2D  IMPLEMENT_SHAPE_FN(TransformedShape, BestEdgeFor,        (const Math::fVector2& normal),           (normal))
    Math::fRange   IMPLEMENT_SHAPE_FN(TransformedShape, ProjectOntoAxis,    (const Math::fVector2& axis),             (axis))
    Math::fRange   IMPLEMENT_SHAPE_FN(TransformedShape, ProjectOntoOwnAxis, (u32 axisID, const Math::fVector2& axis), (axisID, axis))
    bool           IMPLEMENT_SHAPE_FN(TransformedShape, AddSeperatingAxes,  (SeperatingAxisSolver& sat),              (sat))
} // Quasi