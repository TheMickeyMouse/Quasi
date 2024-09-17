#include "Collision2D.h"

#include "Body2D.h"
#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    Manifold CollideShapes(const TransformedShape& s1, const TransformedShape& s2) {
        const Shape::ClipPrimitive prim1 = s1.PreferedPrimitive(),
                                   prim2 = s2.PreferedPrimitive();

#define SHAPE_PRIM_PAIR(X, Y) case Shape::PRIM_##X * 3 + Shape::PRIM_##Y
        switch (prim1 * 3 + prim2) {
            SHAPE_PRIM_PAIR(CIRCLE, CIRCLE):
                return CollideCircles(s1.As<TransformedCircleShape>(), s2.As<TransformedCircleShape>());

            SHAPE_PRIM_PAIR(CIRCLE, POINT):  SHAPE_PRIM_PAIR(CIRCLE, EDGE):
                return CollideCircleShape(s1, s2);

            SHAPE_PRIM_PAIR(POINT,  CIRCLE): SHAPE_PRIM_PAIR(EDGE,   CIRCLE):
                return Manifold::Flip(CollideCircleShape(s2, s1));

            SHAPE_PRIM_PAIR(POINT,  POINT):
                return CollideCurves(s1, s2);

            SHAPE_PRIM_PAIR(EDGE,   POINT):
                return CollidePolygonCurve(s1, s2);

            SHAPE_PRIM_PAIR(POINT,  EDGE):
                return Manifold::Flip(CollidePolygonCurve(s2, s1));

            SHAPE_PRIM_PAIR(EDGE,  EDGE):
                return CollidePolygons(s1, s2);

            default:
                return Manifold::None();
        }
    }

    Manifold CollideCircles(const TransformedCircleShape& s1, const TransformedCircleShape& s2) {
        const float dist = s1.center.dist(s2.center), depth = s1.radius + s2.radius - dist;
        if (depth <= 0)
            return Manifold::None();
        const Math::fVector2 n = (s2.center - s1.center) / dist;
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { s2.center - n * s2.radius },
            .contactDepth = { depth },
            .contactCount = 1,
        };
    }

    Manifold CollideCircleShape(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        const auto& circle = *s1.As<TransformedCircleShape>();

        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(circle.center - s2.NearestPointTo(circle.center));

        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        sat.Finish();

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { circle.center + sat.GetSepAxis() * circle.radius },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygons(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();
        if (!sat.Collides())
            return Manifold::None();

        return Manifold::From(sat);
    }

    Manifold CollideCurves(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);

        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        const bool usesTarget = sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();

        const Math::fVector2& n = sat.GetSepAxis();

        if (!sat.Collides())
            return Manifold::None();

        if (usesTarget) {
            return Manifold {
                .seperatingNormal = n,
                .contactPoint = { s1.FurthestAlong(n) },
                .contactDepth = { sat.GetDepth() },
                .contactCount = 1,
            };
        }
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { s2.FurthestAlong(n) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygonCurve(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.Finish();
        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { s2.FurthestAlong(sat.GetSepAxis()) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    bool OverlapShapes(const TransformedShape& s1, const TransformedShape& s2) {
        const bool circ1 = s1.PreferedPrimitive() == Shape::PRIM_CIRCLE,
                   circ2 = s2.PreferedPrimitive() == Shape::PRIM_CIRCLE;

        if (circ1 && circ2) {
            return OverlapCircles(s1.As<TransformedCircleShape>(), s2.As<TransformedCircleShape>());
        }
        if (circ1) {
            return OverlapCircleShape(s1, s2);
        }
        if (circ2) {
            return OverlapCircleShape(s2, s1);
        }
        return OverlapNonCircles(s1, s2);
    }

    bool OverlapCircles(const TransformedCircleShape& s1, const TransformedCircleShape& s2) {
        return s1.center.in_range(s2.center, s1.radius + s2.radius);
    }

    bool OverlapCircleShape(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, s2);
        const auto& circ = *s1.As<TransformedCircleShape>();
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(circ.center - s2.NearestPointTo(circ.center));
        return sat.Collides();
    }

    bool OverlapNonCircles(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, s2);
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

        const float e = std::min(body.restitution, target.restitution);
        const Math::fVector2 impulse = -(1 + e) * j * normal;

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