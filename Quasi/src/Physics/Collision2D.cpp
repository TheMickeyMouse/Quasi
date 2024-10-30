#include "Collision2D.h"

#include "Body2D.h"
#include "SeperatingAxisSolver.h"

namespace Quasi::Physics2D {
    Manifold CollideShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const Shape::ClipPrimitive prim1 = s1.PreferedPrimitive(),
                                   prim2 = s2.PreferedPrimitive();

#define SHAPE_PRIM_PAIR(X, Y) case Shape::PRIM_##X * 3 + Shape::PRIM_##Y
        switch (prim1 * 3 + prim2) {
            SHAPE_PRIM_PAIR(CIRCLE, CIRCLE):
                return CollideCircles(s1.As<CircleShape>(), xf1, s2.As<CircleShape>(), xf2);

            SHAPE_PRIM_PAIR(CIRCLE, LINE):  SHAPE_PRIM_PAIR(CIRCLE, POLYGON):
                return CollideCircleShape(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(LINE, CIRCLE): SHAPE_PRIM_PAIR(POLYGON, CIRCLE):
                return Manifold::Flip(CollideCircleShape(s2, xf2, s1, xf1));

            SHAPE_PRIM_PAIR(LINE, LINE):
                return CollideCapsules(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(POLYGON, LINE):
                return CollidePolygonCapsule(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(LINE, POLYGON):
                return Manifold::Flip(CollidePolygonCapsule(s2, xf2, s1, xf1));

            SHAPE_PRIM_PAIR(POLYGON, POLYGON):
                return CollidePolygons(s1, xf1, s2, xf2);

            default:
                return Manifold::None();
        }
    }

    Manifold CollideCircles(const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2) {
        float distsq = xf1.position.distsq(xf2.position);
        if (distsq >= (s1.radius + s2.radius) * (s1.radius + s2.radius))
            return Manifold::None();

        distsq = std::sqrt(distsq);
        const fVector2 n = (xf2.position - xf1.position) / distsq;
        return Manifold {
            .seperatingNormal = n,
            .contactPoint = { xf1.position - n * s2.radius },
            .contactDepth = { s1.radius + s2.radius - distsq },
            .contactCount = 1,
        };
    }

    Manifold CollideCircleShape(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        const auto& circle = *s1.As<CircleShape>();

        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        sat.CheckAxis((xf2.Transform(s2.NearestPointTo(xf2.TransformInverse(xf1.position))) - xf1.position).norm());

        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { xf1.position + sat.GetSepAxis() * circle.radius },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygons(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        if (!sat.Collides())
            return Manifold::None();

        return Manifold::From(sat);
    }

    Manifold CollideCapsules(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        const CapsuleShape& cap1 = s1.As<CapsuleShape>(),
                          & cap2 = s2.As<CapsuleShape>();

        const fVector2 off = xf1.position - xf2.position;
        const fVector2 tip1 = off + cap1.forward, end1 = off - cap1.forward,
                       tip2 = off + cap2.forward, end2 = off - cap2.forward;
        const fVector2 axis1 = tip1 - cap2.forward * std::clamp(tip1.dot(cap2.fwdOverLsq), -1.0f, 1.0f),
                       axis2 = end1 - cap2.forward * std::clamp(end1.dot(cap2.fwdOverLsq), -1.0f, 1.0f),
                       axis3 = tip2 - cap1.forward * std::clamp(tip2.dot(cap1.fwdOverLsq), -1.0f, 1.0f),
                       axis4 = end2 - cap1.forward * std::clamp(end2.dot(cap1.fwdOverLsq), -1.0f, 1.0f);
        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        sat.CheckAxis(axis1.norm());
        sat.CheckAxis(axis2.norm());
        sat.CheckAxis(axis3.norm());
        sat.CheckAxis(axis4.norm());

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { s2.FurthestAlong(-sat.GetSepAxis()) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    Manifold CollidePolygonCapsule(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        const auto& cap = *s2.As<CapsuleShape>();
        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        const fVector2 tip = xf2.Transform(cap.forward), end = 2 * xf2.position - tip;
        sat.CheckAxis((xf1.Transform(s1.NearestPointTo(xf1.TransformInverse(tip))) - tip).norm());
        sat.CheckAxis((xf1.Transform(s1.NearestPointTo(xf1.TransformInverse(end))) - end).norm());

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = { xf2.Transform(cap.FurthestAlong(xf2.TransformInverseOffset(-sat.GetSepAxis()))) },
            .contactDepth = { sat.GetDepth() },
            .contactCount = 1,
        };
    }

    bool OverlapShapes(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const Shape::ClipPrimitive prim1 = s1.PreferedPrimitive(),
                                   prim2 = s2.PreferedPrimitive();

        switch (prim1 * 3 + prim2) {
            SHAPE_PRIM_PAIR(CIRCLE, CIRCLE):
                return OverlapCircles(s1.As<CircleShape>(), xf1, s2.As<CircleShape>(), xf2);

            SHAPE_PRIM_PAIR(CIRCLE, LINE):  SHAPE_PRIM_PAIR(CIRCLE, POLYGON):
                return OverlapCircleShape(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(LINE, CIRCLE): SHAPE_PRIM_PAIR(POLYGON, CIRCLE):
                return OverlapCircleShape(s2, xf2, s1, xf1);

            SHAPE_PRIM_PAIR(LINE, LINE):
                return OverlapCapsules(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(POLYGON, LINE):
                return OverlapPolygonCapsule(s1, xf1, s2, xf2);

            SHAPE_PRIM_PAIR(LINE, POLYGON):
                return OverlapPolygonCapsule(s2, xf2, s1, xf1);

            SHAPE_PRIM_PAIR(POLYGON, POLYGON):
                return OverlapPolygons(s1, xf1, s2, xf2);

            default:
                return false;
        }
    }

    bool OverlapCircles(const CircleShape& s1, const PhysicsTransform& xf1, const CircleShape& s2, const PhysicsTransform& xf2) {
        return xf1.position.in_range(xf2.position, s1.radius + s2.radius);
    }

    bool OverlapCircleShape(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckOverlapFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);
        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        sat.CheckAxis((xf2.Transform(s2.NearestPointTo(xf2.TransformInverse(xf1.position))) - xf1.position).norm());
        return sat.Collides();
    }

    bool OverlapPolygons(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        return sat.Collides();
    }

    bool OverlapCapsules(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        const CapsuleShape& cap1 = s1.As<CapsuleShape>(),
                          & cap2 = s2.As<CapsuleShape>();
        const auto [p1, p2] = fLine2D         { xf1.position - cap1.forward, xf1.position + cap1.forward }
                             .nearest_between({ xf2.position - cap2.forward, xf2.position + cap2.forward });
        return p1.in_range(p2, cap1.radius + cap2.radius);
    }

    bool OverlapPolygonCapsule(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
        sat.CheckAxisFor(SeperatingAxisSolver::BASE);
        sat.CheckAxisFor(SeperatingAxisSolver::TARGET);

        const auto& cap = *s2.As<CapsuleShape>();
        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
        const fVector2 tip = xf2.Transform(cap.forward), end = 2 * xf2.position - tip;
        sat.CheckAxis((xf1.Transform(s1.NearestPointTo(xf1.TransformInverse(tip))) - tip).norm());
        sat.CheckAxis((xf1.Transform(s1.NearestPointTo(xf1.TransformInverse(end))) - end).norm());

        return sat.Collides();
    }

    void StaticResolve(Body& body, Body& target, const Manifold& manifold) {
        // if (manifold.flipped)
        //     std::swap(body, target);
        const bool bodyDyn   = body.IsDynamic(),
                   targetDyn = target.IsDynamic(),
                   shareForce = bodyDyn && targetDyn;
        fVector2 sep = manifold.seperatingNormal;
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
        const fVector2& normal = manifold.seperatingNormal, tangent = normal.perpend();
        const float totalInvMass = body.invMass + target.invMass;
        const float share = manifold.contactCount == 2 ? 0.5f : 1.0f;

        // if (relativeVelocity.dot(normal) > 0.0f) { // moving away from each other, dont collide
        //     return;
        // }

        /*
         *  // FlatBody bodyA = contact.BodyA;
            // FlatBody bodyB = contact.BodyB;
            // FlatVector normal = contact.Normal;
            // FlatVector contact1 = contact.Contact1;
            // FlatVector contact2 = contact.Contact2;
            // int contactCount = contact.ContactCount;

            // float e = MathF.Min(bodyA.Restitution, bodyB.Restitution);

            float sf = (bodyA.StaticFriction + bodyB.StaticFriction) * 0.5f;
            float df = (bodyA.DynamicFriction + bodyB.DynamicFriction) * 0.5f;

            this.contactList[0] = contact1;
            this.contactList[1] = contact2;

            for (int i = 0; i < contactCount; i++)
            {
                this.impulseList[i] = FlatVector.Zero;
                this.raList[i] = FlatVector.Zero;
                this.rbList[i] = FlatVector.Zero;
                this.frictionImpulseList[i] = FlatVector.Zero;
                this.jList[i] = 0f;
            }

            for (int i = 0; i < contactCount; i++)
            {
                FlatVector ra = contactList[i] - bodyA.Position;
                FlatVector rb = contactList[i] - bodyB.Position;

                raList[i] = ra;
                rbList[i] = rb;

                FlatVector raPerp = new FlatVector(-ra.Y, ra.X);
                FlatVector rbPerp = new FlatVector(-rb.Y, rb.X);

                FlatVector angularLinearVelocityA = raPerp * bodyA.AngularVelocity;
                FlatVector angularLinearVelocityB = rbPerp * bodyB.AngularVelocity;

                FlatVector relativeVelocity =
                    (bodyB.LinearVelocity + angularLinearVelocityB) -
                    (bodyA.LinearVelocity + angularLinearVelocityA);

                float contactVelocityMag = FlatMath.Dot(relativeVelocity, normal);

                if (contactVelocityMag > 0f)
                {
                    continue;
                }

                float raPerpDotN = FlatMath.Dot(raPerp, normal);
                float rbPerpDotN = FlatMath.Dot(rbPerp, normal);

                float denom = bodyA.InvMass + bodyB.InvMass +
                    (raPerpDotN * raPerpDotN) * bodyA.InvInertia +
                    (rbPerpDotN * rbPerpDotN) * bodyB.InvInertia;

                float j = -(1f + e) * contactVelocityMag;
                j /= denom;
                j /= (float)contactCount;

                jList[i] = j;

                FlatVector impulse = j * normal;
                impulseList[i] = impulse;
            }

            for (int i = 0; i < contactCount; i++)
            {
                FlatVector impulse = impulseList[i];
                FlatVector ra = raList[i];
                FlatVector rb = rbList[i];

                bodyA.LinearVelocity += -impulse * bodyA.InvMass;
                bodyA.AngularVelocity += -FlatMath.Cross(ra, impulse) * bodyA.InvInertia;
                bodyB.LinearVelocity += impulse * bodyB.InvMass;
                bodyB.AngularVelocity += FlatMath.Cross(rb, impulse) * bodyB.InvInertia;
            }

            for (int i = 0; i < contactCount; i++)
            {
                FlatVector ra = contactList[i] - bodyA.Position;
                FlatVector rb = contactList[i] - bodyB.Position;

                raList[i] = ra;
                rbList[i] = rb;

                FlatVector raPerp = new FlatVector(-ra.Y, ra.X);
                FlatVector rbPerp = new FlatVector(-rb.Y, rb.X);

                FlatVector angularLinearVelocityA = raPerp * bodyA.AngularVelocity;
                FlatVector angularLinearVelocityB = rbPerp * bodyB.AngularVelocity;

                FlatVector relativeVelocity =
                    (bodyB.LinearVelocity + angularLinearVelocityB) -
                    (bodyA.LinearVelocity + angularLinearVelocityA);

                FlatVector tangent = relativeVelocity - FlatMath.Dot(relativeVelocity, normal) * normal;

                if(FlatMath.NearlyEqual(tangent, FlatVector.Zero))
                {
                    continue;
                }
                else
                {
                    tangent = FlatMath.Normalize(tangent);
                }

                float raPerpDotT = FlatMath.Dot(raPerp, tangent);
                float rbPerpDotT = FlatMath.Dot(rbPerp, tangent);

                float denom = bodyA.InvMass + bodyB.InvMass +
                    (raPerpDotT * raPerpDotT) * bodyA.InvInertia +
                    (rbPerpDotT * rbPerpDotT) * bodyB.InvInertia;

                float jt = -FlatMath.Dot(relativeVelocity, tangent);
                jt /= denom;
                jt /= (float)contactCount;

                FlatVector frictionImpulse;
                float j = jList[i];

                if(MathF.Abs(jt) <= j * sf)
                {
                    frictionImpulse = jt * tangent;
                }
                else
                {
                    frictionImpulse = -j * tangent * df;
                }

                this.frictionImpulseList[i] = frictionImpulse;
            }

            for (int i = 0; i < contactCount; i++)
            {
                FlatVector frictionImpulse = this.frictionImpulseList[i];
                FlatVector ra = raList[i];
                FlatVector rb = rbList[i];

                bodyA.LinearVelocity += -frictionImpulse * bodyA.InvMass;
                bodyA.AngularVelocity += -FlatMath.Cross(ra, frictionImpulse) * bodyA.InvInertia;
                bodyB.LinearVelocity += frictionImpulse * bodyB.InvMass;
                bodyB.AngularVelocity += FlatMath.Cross(rb, frictionImpulse) * bodyB.InvInertia;
            }
         *
         */

        constexpr float BODY_RESTITUION = 0.4f;
        constexpr float STATIC_FRICTION = 0.75f, DYNAMIC_FRICTION = 0.57f;

        fVector2 relBody[2],     relTarget[2];
        float    velocityMag[2], jn[2];

        fVector2 totalImpulse = 0;
        float    totalBodyAngMtm = 0, totalTargetAngMtm = 0;
        // normal forces
        for (u32 i = 0; i < manifold.contactCount; ++i) {
            const fVector2& contact = manifold.contactPoint[i];

            relBody  [i] = contact - body.position,
            relTarget[i] = contact - target.position;

            const fVector2 angularVelBody   = relBody  [i].perpend() * -body.angularVelocity,
                           angularVelTarget = relTarget[i].perpend() * -target.angularVelocity;

            const fVector2 relVel = (target.velocity + angularVelTarget) -
                                    (body  .velocity + angularVelBody);

            velocityMag[i] = relVel.dot(normal);
            if (velocityMag[i] > 0.0f) continue;

            const float torqueRadiusNBody   = relBody  [i].zcross(normal),
                        torqueRadiusNTarget = relTarget[i].zcross(normal);

            const float denomN = totalInvMass +
                                (torqueRadiusNBody   * torqueRadiusNBody)   * body  .invInertia +
                                (torqueRadiusNTarget * torqueRadiusNTarget) * target.invInertia;

            jn[i] = -(1.0f + BODY_RESTITUION) * velocityMag[i] * share / denomN;

            const fVector2 impulse = jn[i] * normal;

            totalImpulse      += impulse;
            totalBodyAngMtm   -= relBody  [i].zcross(impulse);
            totalTargetAngMtm += relTarget[i].zcross(impulse);
        }

        body.AddMomentum(-totalImpulse);
        body.AddAngularMomentum(totalBodyAngMtm);
        target.AddMomentum(totalImpulse);
        target.AddAngularMomentum(totalTargetAngMtm);

        totalImpulse = 0;
        totalBodyAngMtm = 0; totalTargetAngMtm = 0;
        // frictional forces (needs to be separated, order matters)
        for (u32 i = 0; i < manifold.contactCount; ++i) {
            const fVector2 angularVelBody   = relBody  [i].perpend() * -body.angularVelocity,
                           angularVelTarget = relTarget[i].perpend() * -target.angularVelocity;

            const fVector2 relVel = (target.velocity + angularVelTarget) -
                                    (body  .velocity + angularVelBody);

            const float t = relVel.dot(tangent);

            if (std::abs(t) <= EPSILON) continue;

            const float torqueRadiusTBody   = relBody  [i].zcross(tangent),
                        torqueRadiusTTarget = relTarget[i].zcross(tangent);

            const float denomT = totalInvMass +
                                (torqueRadiusTBody   * torqueRadiusTBody)   * body  .invInertia +
                                (torqueRadiusTTarget * torqueRadiusTTarget) * target.invInertia;

            const float jt = -t * share / denomT;
            const fVector2 frictionImpulse = tangent * (
                std::abs(jt) <= std::abs(jn[i] * STATIC_FRICTION) ?
                jt : (jt < 0 ? -1.0f : 1.0f) * jn[i] * DYNAMIC_FRICTION
            ); // static/dynamic friction picking

            totalImpulse      += frictionImpulse;
            totalBodyAngMtm   -= relBody  [i].zcross(frictionImpulse);
            totalTargetAngMtm += relTarget[i].zcross(frictionImpulse);
        }

        body.AddMomentum(-totalImpulse);
        body.AddAngularMomentum(totalBodyAngMtm);
        target.AddMomentum(totalImpulse);
        target.AddAngularMomentum(totalTargetAngMtm);
    }
} // Physics2D