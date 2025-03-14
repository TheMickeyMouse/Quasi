#pragma once
#include "Manifold2D.h"
#include "Math/Vector.h"

namespace Quasi::Physics2D {
    class Shape;
    class CircleShape;
    class CapsuleShape;
    class Body;
}

namespace Quasi::Physics2D {
    float ClosestBetweenSegments(const fVector2& a1, const fVector2& b1, const fVector2& a2, const fVector2& b2,
                                 float* s, float* t, fVector2* c1, fVector2* c2);

    Manifold CollideShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

    Manifold CollideCircles       (const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2);
    Manifold CollideCircleShape   (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollidePolygons      (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollideCapsules      (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    Manifold CollidePolygonCapsule(const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);

    bool OverlapShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

    bool OverlapCircles       (const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2);
    bool OverlapCircleShape   (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    bool OverlapPolygons      (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    bool OverlapCapsules      (const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);
    bool OverlapPolygonCapsule(const Shape& s1,       const PhysicsTransform& xf1, const Shape& s2,       const PhysicsTransform& xf2);

    void StaticResolve (Body& body, Body& target, const Manifold& manifold);
    template <u32 ContactCount, bool BDyn, bool TDyn>
    void DynamicResolveFor(Body& body, Body& target, const Manifold& manifold);
    void DynamicResolve(Body& body, Body& target, const Manifold& manifold);
} // Physics2D::Collision
