#pragma once
#include "Manifold2D.h"
#include "PhysicsTransform2D.h"

namespace Quasi::Physics2D {
    class CircleShape;
    class Shape;
    class Body;
}

namespace Quasi::Physics2D {
    Manifold CollideShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

    Manifold CollideCircles     (const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2);
    Manifold CollideCircleShape (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollidePolygons    (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollideCurves      (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollidePolygonCurve(const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);

    bool OverlapShapes     (const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

    bool OverlapCircles    (const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2);
    bool OverlapCircleShape(const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    bool OverlapNonCircles (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);

    void StaticResolve (Body& body, Body& target, const Manifold& manifold);
    void DynamicResolve(Body& body, Body& target, const Manifold& manifold);
} // Physics2D::Collision
