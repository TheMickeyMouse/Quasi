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

            SHAPE_PRIM_PAIR(CIRCLE, LINE):  SHAPE_PRIM_PAIR(CIRCLE, POLYGON):
                return CollideCircleShape(s1, s2);

            SHAPE_PRIM_PAIR(LINE, CIRCLE): SHAPE_PRIM_PAIR(POLYGON, CIRCLE):
                return Manifold::Flip(CollideCircleShape(s2, s1));

            SHAPE_PRIM_PAIR(LINE, LINE):
                return CollideCapsules(s1, s2);

            SHAPE_PRIM_PAIR(POLYGON, LINE):
                return CollidePolygonCapsule(s1, s2);

            SHAPE_PRIM_PAIR(LINE, POLYGON):
                return Manifold::Flip(CollidePolygonCapsule(s2, s1));

            SHAPE_PRIM_PAIR(POLYGON, POLYGON):
                return CollidePolygons(s1, s2);

            default:
                return Manifold::None();
        }
    }

    Manifold CollideCircles(const TransformedCircleShape& s1, const TransformedCircleShape& s2) {
        float distsq = s1.center.distsq(s2.center);
        if (distsq >= (s1.radius + s2.radius) * (s1.radius + s2.radius))
            return Manifold::None();

        distsq = std::sqrt(distsq);
        const Math::fVector2 n = (s2.center - s1.center) / distsq;
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { s2.center - n * s2.radius },
            .contactDepth = { s1.radius + s2.radius - distsq },
            .contactCount = 1,
        };
    }

    Manifold CollideCircleShape(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        const auto& circle = *s1.As<TransformedCircleShape>();

        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(s2.NearestPointTo(circle.center) - circle.center);

        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

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

        if (!sat.Collides())
            return Manifold::None();

        return Manifold::From(sat);
    }

    Manifold CollideCapsules(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        const TransformedCapsuleShape& cap1 = s1.As<TransformedCapsuleShape>(),
                                     & cap2 = s2.As<TransformedCapsuleShape>();

        const Math::fVector2 invFwd1 = cap1.forward / cap1.forward.lensq(), invFwd2 = cap2.forward / cap2.forward.lensq();
        const Math::fVector2 off = cap1.start - cap2.start;
        const Math::fVector2 axis1 =  off                - cap2.forward * std::clamp( off                .dot(invFwd2), 0.0f, 1.0f),
                             axis2 =  off + cap1.forward - cap2.forward * std::clamp((off + cap1.forward).dot(invFwd2), 0.0f, 1.0f),
                             axis3 = -off                + cap1.forward * std::clamp( off                .dot(invFwd1), 0.0f, 1.0f),
                             axis4 = -off + cap2.forward + cap1.forward * std::clamp((off + cap2.forward).dot(invFwd1), 0.0f, 1.0f);
        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        sat.CheckAxis(axis1);
        sat.CheckAxis(axis2);
        sat.CheckAxis(axis3);
        sat.CheckAxis(axis4);

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { s2.FurthestAlong(-sat.GetSepAxis()) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygonCapsule(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

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
        const Shape::ClipPrimitive prim1 = s1.PreferedPrimitive(),
                                   prim2 = s2.PreferedPrimitive();

        switch (prim1 * 3 + prim2) {
            SHAPE_PRIM_PAIR(CIRCLE, CIRCLE):
                return OverlapCircles(s1.As<TransformedCircleShape>(), s2.As<TransformedCircleShape>());

            SHAPE_PRIM_PAIR(CIRCLE, LINE):  SHAPE_PRIM_PAIR(CIRCLE, POLYGON):
                return OverlapCircleShape(s1, s2);

            SHAPE_PRIM_PAIR(LINE, CIRCLE): SHAPE_PRIM_PAIR(POLYGON, CIRCLE):
                return OverlapCircleShape(s2, s1);

            SHAPE_PRIM_PAIR(LINE, LINE):
                return OverlapCapsules(s1, s2);

            SHAPE_PRIM_PAIR(POLYGON, LINE):
                return OverlapPolygonCapsule(s1, s2);

            SHAPE_PRIM_PAIR(LINE, POLYGON):
                return OverlapPolygonCapsule(s2, s1);

            SHAPE_PRIM_PAIR(POLYGON, POLYGON):
                return OverlapPolygons(s1, s2);

            default:
                return false;
        }
    }

    bool OverlapCircles(const TransformedCircleShape& s1, const TransformedCircleShape& s2) {
        return s1.center.in_range(s2.center, s1.radius + s2.radius);
    }

    bool OverlapCircleShape(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, s2);
        const auto& circ = *s1.As<TransformedCircleShape>();
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.SetCheckFor(SeperatingAxisSolver::BASE);
        sat.CheckAxis(s2.NearestPointTo(circ.center) - circ.center);
        return sat.Collides();
    }

    bool OverlapPolygons(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        return sat.Collides();
    }

    bool OverlapCapsules(const TransformedShape& s1, const TransformedShape& s2) {
        const TransformedCapsuleShape& cap1 = s1.As<TransformedCapsuleShape>(),
                                     & cap2 = s2.As<TransformedCapsuleShape>();
        const auto [p1, p2] = Math::fLine2D   { cap1.start, cap1.start + cap1.forward }
                             .nearest_between({ cap2.start, cap2.start + cap2.forward });
        return p1.in_range(p2, cap1.radius + cap2.radius);
    }

    bool OverlapPolygonCapsule(const TransformedShape& s1, const TransformedShape& s2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, s2);
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
        // if (relativeVelocity.dot(normal) > 0.0f) { // moving away from each other, dont collide
        //     return;
        // }

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