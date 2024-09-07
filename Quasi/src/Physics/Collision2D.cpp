#include "Collision2D.h"

#include "Body2D.h"
#include "Geometry.h"
#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D::Collision {
    Manifold CollideShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const Shape::ClipPrimitive prim1 = s1.PreferedClipPrimitive(),
                                   prim2 = s2.PreferedClipPrimitive();

#define SHAPE_PRIM_PAIR(X, Y) case Shape::X * 3 + Shape::Y
        switch (prim1 * 3 + prim2) {
            SHAPE_PRIM_PAIR(CIRCLE, CIRCLE):
                return CollideCircles(Refer(s1).As<CircleShape>(), xf1, Refer(s2).As<CircleShape>(), xf2);

            SHAPE_PRIM_PAIR(CIRCLE, POINT):  SHAPE_PRIM_PAIR(CIRCLE, EDGE):
                return CollideCircleShape(Refer(s1).As<CircleShape>(), xf1, s2, xf2);

            SHAPE_PRIM_PAIR(POINT,  CIRCLE): SHAPE_PRIM_PAIR(EDGE,   CIRCLE):
                return Manifold::Flip(CollideCircleShape(Refer(s2).As<CircleShape>(), xf2, s1, xf1));

            SHAPE_PRIM_PAIR(POINT,  POINT):
                return CollideCurves(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(EDGE,   POINT):
                return CollidePolygonCurve(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(POINT,  EDGE):
                return Manifold::Flip(CollidePolygonCurve(s2, xf2, s1, xf1));

            SHAPE_PRIM_PAIR(EDGE,  EDGE):
                return CollidePolygons(s1, xf1, s2, xf2);

            default:
                return Manifold::None();
        }
    }

    Manifold CollideCircles(const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2) {
        const float dist = xf1.position.dist(xf2.position), depth = s1.radius + s2.radius - dist;
        if (depth <= 0)
            return Manifold::None();
        const Math::fVector2 n = (xf2.position - xf1.position) / dist;
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { xf2.position - n * s2.radius },
            .contactDepth = { depth },
            .contactCount = 1,
        };
    }

    Manifold CollideCircleShape(const CircleShape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const Math::fVector2& center = xf1.position;
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);

        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(center - s2.NearestPointTo(center, xf2));

        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        sat.Finish();

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { center + sat.GetSepAxis() * s1.radius },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygons(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();
        if (!sat.Collides())
            return Manifold::None();

        return Manifold::From(sat);
    }

    Manifold CollideCurves(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);

        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        const bool usesTarget = sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();

        const Math::fVector2& n = sat.GetSepAxis();

        if (!sat.Collides())
            return Manifold::None();

        if (usesTarget) {
            return Manifold {
                .seperatingNormal = n,
                .contactPoint = { s1.FurthestAlong(n, xf1) },
                .contactDepth = { sat.GetDepth() },
                .contactCount = 1,
            };
        }
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { s2.FurthestAlong(n, xf2) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygonCurve(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();
        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { s2.FurthestAlong(sat.GetSepAxis(), xf2) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    bool OverlapShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const bool circ1 = s1.PreferedClipPrimitive() == Shape::CIRCLE,
                   circ2 = s2.PreferedClipPrimitive() == Shape::CIRCLE;

        if (circ1 && circ2) {
            return OverlapCircles(Refer(s1).As<CircleShape>(), xf1, Refer(s2).As<CircleShape>(), xf2);
        }
        if (circ1) {
            return OverlapCircleShape(Refer(s1).As<CircleShape>(), xf1, s2, xf2);
        }
        if (circ2) {
            return OverlapCircleShape(Refer(s2).As<CircleShape>(), xf2, s1, xf1);
        }
        return OverlapNonCircles(s1, xf1, s2, xf2);
    }

    bool OverlapCircles(const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2) {
        return xf1.position.in_range(xf2.position, s1.radius + s2.radius);
    }

    bool OverlapCircleShape(const CircleShape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(xf1.position - s2.NearestPointTo(xf1.position, xf2));
        return sat.Collides();
    }

    bool OverlapNonCircles(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        return sat.Collides();
    }

    void StaticResolve(Body& body, Body& target, const Manifold& manifold) {
        // if (manifold.flipped)
        //     std::swap(body, target);
        const bool bodyDyn   = body.IsDynamic(),
                   targetDyn = target.IsDynamic(),
                   shareForce = bodyDyn && targetDyn;
        Math::fVector2 sep = manifold.seperatingNormal;
        switch (manifold.contactCount) {
            case 0: return;
            case 1: {
                sep *= shareForce ? 0.5f * manifold.contactDepth[0] : manifold.contactDepth[0];
                if (bodyDyn)
                    body.position -= sep;
                if (targetDyn)
                    target.position += sep;
                break;
            }
            case 2: {
                const float depth = std::max(manifold.contactDepth[0], manifold.contactDepth[1]);
                sep *= shareForce ? 0.5f * depth : depth;
                if (bodyDyn)
                    body.position -= sep;
                if (targetDyn)
                    target.position += sep;
            }
            default: return;
        }
    }

    void DynamicResolve(Body& body, Body& target, const Manifold& manifold) {
        const Math::fVector2& normal = manifold.seperatingNormal;
        const Math::fVector2 relativeVelocity = target.velocity - body.velocity;
        const bool bodyDyn = body.IsDynamic(), targetDyn = target.IsDynamic();

        const float j = relativeVelocity.dot(normal);
        if (relativeVelocity.dot(normal) > 0.0f) { // moving away from each other, dont collide
            return;
        }

        const Math::fVector2 impulse = -2.0f * j * normal;

        if (bodyDyn && targetDyn) {
            const float invTotalMass = 1.0f / (body.mass + target.mass);
            body.velocity   -= impulse * (target.mass * invTotalMass);
            target.velocity += impulse * (body.mass * invTotalMass);
        } else if (bodyDyn) {
            body.velocity   -= impulse;
        } else if (targetDyn) {
            target.velocity += impulse;
        }
    }
} // Physics2D