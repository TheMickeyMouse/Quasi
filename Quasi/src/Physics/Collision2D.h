#pragma once
#include "Manifold2D.h"

namespace Quasi::Physics2D {
    class TransformedCircleShape;
    class TransformedCapsuleShape;
    class TransformedShape;
    class Body;
}

namespace Quasi::Physics2D {
    Manifold CollideShapes(const TransformedShape& s1, const TransformedShape& s2);

    Manifold CollideCircles       (const TransformedCircleShape& s1,  const TransformedCircleShape& s2);
    Manifold CollideCircleShape   (const TransformedShape& s1,        const TransformedShape& s2);
    Manifold CollidePolygons      (const TransformedShape& s1,        const TransformedShape& s2);
    Manifold CollideCapsules      (const TransformedShape& s1,        const TransformedShape& s2);
    Manifold CollidePolygonCapsule(const TransformedShape& s1,        const TransformedShape& s2);

    bool OverlapShapes     (const TransformedShape& s1, const TransformedShape& s2);

    bool OverlapCircles       (const TransformedCircleShape& s1, const TransformedCircleShape& s2);
    bool OverlapCircleShape   (const TransformedShape& s1,        const TransformedShape& s2);
    bool OverlapPolygons      (const TransformedShape& s1,        const TransformedShape& s2);
    bool OverlapCapsules      (const TransformedShape& s1,        const TransformedShape& s2);
    bool OverlapPolygonCapsule(const TransformedShape& s1,        const TransformedShape& s2);

    void StaticResolve (Body& body, Body& target, const Manifold& manifold);
    void DynamicResolve(Body& body, Body& target, const Manifold& manifold);
} // Physics2D::Collision
