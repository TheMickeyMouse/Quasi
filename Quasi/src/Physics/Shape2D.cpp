#include "Shape2D.h"

namespace Quasi::Physics2D {
#define IMPLEMENT_SHAPE_FN(R, K, N, PARAMS, ARGS) R K::N PARAMS const { return Visit<R>([&] (const auto& s) { return s.N ARGS; }); }

    IMPLEMENT_SHAPE_FN(float,    Shape, ComputeArea,        (), ())
    IMPLEMENT_SHAPE_FN(fRect2D,  Shape, ComputeBoundingBox, (), ())
    IMPLEMENT_SHAPE_FN(float,    Shape, Inertia,            (), ())
    IMPLEMENT_SHAPE_FN(fVector2, Shape, NearestPointTo,     (const fVector2& point),            (point))
    IMPLEMENT_SHAPE_FN(fVector2, Shape, FurthestAlong,      (const fVector2& normal),           (normal))
    IMPLEMENT_SHAPE_FN(fLine2D,  Shape, BestEdgeFor,        (const fVector2& normal),           (normal))
    IMPLEMENT_SHAPE_FN(fRange,   Shape, ProjectOntoAxis,    (const fVector2& axis),             (axis))
    IMPLEMENT_SHAPE_FN(fRange,   Shape, ProjectOntoOwnAxis, (u32 axisID, const fVector2& axis), (axisID, axis))
    IMPLEMENT_SHAPE_FN(bool,     Shape, AddSeperatingAxes,  (SeperatingAxisSolver& sat),        (sat))

    Shape MakePolygon(Span<const fVector2> points) {
        switch (points.Length()) {
            case 3:  return TriangleShape   { points };
            case 4:  return QuadShape       { points };
            default: return DynPolygonShape { points };
        }
    }
} // Quasi