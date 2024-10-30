#include "Shape2D.h"

namespace Quasi::Physics2D {
#define IMPLEMENT_SHAPE_FN(K, N, PARAMS, ARGS) K::N PARAMS const { return Visit([&] (const auto& s) { return s.N ARGS; }); }

    float    IMPLEMENT_SHAPE_FN(Shape, ComputeArea,        (), ())
    fRect2D  IMPLEMENT_SHAPE_FN(Shape, ComputeBoundingBox, (), ())
    fVector2 IMPLEMENT_SHAPE_FN(Shape, CenterOfMass,       (), ())
    float    IMPLEMENT_SHAPE_FN(Shape, Inertia,            (), ())
    fVector2 IMPLEMENT_SHAPE_FN(Shape, NearestPointTo,     (const fVector2& point),            (point))
    fVector2 IMPLEMENT_SHAPE_FN(Shape, FurthestAlong,      (const fVector2& normal),           (normal))
    fLine2D  IMPLEMENT_SHAPE_FN(Shape, BestEdgeFor,        (const fVector2& normal),           (normal))
    fRange   IMPLEMENT_SHAPE_FN(Shape, ProjectOntoAxis,    (const fVector2& axis),             (axis))
    fRange   IMPLEMENT_SHAPE_FN(Shape, ProjectOntoOwnAxis, (u32 axisID, const fVector2& axis), (axisID, axis))
    bool     IMPLEMENT_SHAPE_FN(Shape, AddSeperatingAxes,  (SeperatingAxisSolver& sat),        (sat))
} // Quasi