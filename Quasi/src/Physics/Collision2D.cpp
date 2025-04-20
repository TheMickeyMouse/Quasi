#include "Collision2D.h"

#include <bits/stl_algo.h>

#include "Body2D.h"
#include "SeperatingAxisSolver.h"
#include "Debug/Logger.h"
#include "Math/Geometry.h"

namespace Quasi::Physics2D {
    float ClosestBetweenSegments(const fVector2& a1, const fVector2& b1, const fVector2& a2, const fVector2& b2,
                                 float* s, float* t, fVector2* c1, fVector2* c2) {
        const fVector2 direction1 = b1 - a1; // Direction vector of segment S1
        const fVector2 direction2 = b2 - a2; // Direction vector of segment S2
        const fVector2 relative = a1 - a2;
        const float len1 = direction1.lensq(); // Squared length of segment S1, always nonnegative
        const float len2 = direction2.lensq(); // Squared length of segment S2, always nonnegative
        const float r2 = direction2.dot(relative);

        // Check if either or both segments degenerate into points
        if (len1 <= f32s::EPSILON && len2 <= f32s::EPSILON) {
            // Both segments degenerate into points
            *s = *t = 0.0f;
            *c1 = a1;
            *c2 = a2;
            return c1->distsq(*c2);
        }
        if (len1 <= f64s::EPSILON) {
            // First segment degenerates into a point
            *s = 0.0f;
            *t = r2 / len2; // s = 0 => t = (b*s + f) / e = f / e
            *t = std::clamp(*t, 0.0f, 1.0f);
        } else {
            const float r1 = direction1.dot(relative);
            if (len2 <= f64s::EPSILON) {
                // Second segment degenerates into a point
                *t = 0.0f;
                *s = std::clamp(-r1 / len1, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
            } else {
                // The general nondegenerate case starts here
                const float b = direction1.dot(direction2);
                const float denom = len1 * len2 - b * b; // Always nonnegative
                // If segments not parallel, compute closest point on L1 to L2 and
                // clamp to segment S1. Else pick arbitrary s (here 0)
                if (denom != 0.0f) {
                    *s = std::clamp((b * r2 - r1 * len2) / denom, 0.0f, 1.0f);
                } else *s = 0.0f;
                // Compute point on L2 closest to S1(s) using
                // t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
                *t = (b * *s + r2) / len2;
                // If t in [0,1] done. Else clamp t, recompute s for the new value
                // of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
                // and clamp s to [0, 1]
                if (*t < 0.0f) {
                    *t = 0.0f;
                    *s = std::clamp(-r1 / len1, 0.0f, 1.0f);
                } else if (*t > 1.0f) {
                    *t = 1.0f;
                    *s = std::clamp((b - r1) / len1, 0.0f, 1.0f);
                }
            }
        }
        *c1 = a1 + direction1 * *s;
        *c2 = a2 + direction2 * *t;
        return c1->distsq(*c2);
    }

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
        const CapsuleShape& cap1 = s1.As<CapsuleShape>(),
                          & cap2 = s2.As<CapsuleShape>();

        // from cap2 local space to cap1 local space
  //       const fVector2 f1  = xf1.TransformDir(cap1.forward),
  //                      a1  = xf1.position - f1,
  //   				   b1  = xf1.position + f1,
  //                      f2  = xf2.TransformDir(cap2.forward),
  //                      a2  = xf2.position - f2,
  //                      b2  = xf2.position + f2,
  //                      rel = a1 - a2;
  //
  //       const float lensq1 = f1.lensq(), lensq2 = f2.lensq();
  //       const float k1 = rel.dot(f1), k2 = rel.dot(f2), dboth = f1.dot(f2);
  //       const float denom = lensq1 * lensq2 - dboth * dboth;
  //
  //       // fraction on segment 1
  //       float u = 0.0f;
  //       if (denom != 0.0f) {
  //           // not parallel
  //           u = std::clamp((dboth * k2 - k1 * lensq2) / denom, 0.0f, 2.0f);
  //       }
  //
  //       // Compute point on segment 2 closest to p1 + f1 * d1
  //       float v = (dboth * u + k2) / lensq2;
  //
  //       // Clamping of segment 2 requires a do over on segment 1
  //       if (v < 0.0f) {
  //           v = 0.0f;
  //           u = std::clamp((0 - k1) / lensq1, 0.0f, 2.0f);
  //       } else if (v > 2.0f) {
  //           v = 2.0f;
  //           u = std::clamp((2 * dboth - k1) / lensq1, 0.0f, 2.0f);
  //       }
  //
  //       const fVector2 closest1 = a1 + f1 * u;
  //       const fVector2 closest2 = a2 + f2 * v;
  //       const float distSq = closest1.distsq(closest2);
  //
  //
		// Manifold manifold = { 0 };
		// float totalRadius = cap1.radius + cap2.radius;
  //
		// if (distSq > totalRadius * totalRadius) {
		// 	return manifold;
		// }
  //
		// const float dist = std::sqrt(distSq);
		// const float len1 = std::sqrt(lensq1), len2 = std::sqrt(lensq2);
  //   	const fVector2 fn1 = f1 / len1, fn2 = f2 / len2;
  //
		// // Does cap2 project outside cap1?
		// const float projA2 = (a2 - a1).dot(fn1),
  //   				projB2 = (b2 - a1).dot(fn1);
		// const bool outsideA = (projA2 <= 0.0f && projB2 <= 0.0f) || (projA2 >= 2 * len1 && projB2 >= 2 * len1);
  //
		// // Does cap1 project outside cap2?
  //   	const float projA1 = (a1 - a2).dot(fn2),
		// 			projB1 = (b1 - a2).dot(fn2);
		// const bool outsideB = (projA1 <= 0.0f && projB1 <= 0.0f) || (projA1 >= 2 * len2 && projB1 >= 2 * len2);
  //
		// if (!outsideA && !outsideB) {
		// 	// attempt to clip
		// 	// this may yield contact points with excessive separation
		// 	// in that case the algorithm falls back to single point collision
  //
		// 	// find reference edge using SAT
		// 	SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);
		// 	sat.SetCheckFor(SeperatingAxisSolver::NEITHER);
		// 	sat.CheckAxis(fn1.perpend());
		// 	const bool useCap2 = sat.CheckAxis(fn2.perpend());
  //
		// 	manifold.seperatingNormal = sat.GetSepAxis();
		// 	if (!useCap2) {
		// 		const fVector2& n = sat.GetSepAxis();
		// 		fVector2 contactLeft = a2, contactRight = b2;
		// 		// clip leftwise
		// 		if (projA2 < 0 && projB2 > 0) {
		// 			contactLeft  = a2 + f2 * ((0.0f - projA2) / (projB2 - projA2));
		// 		} else if (projA2 > 0 && projB2 < 0) {
		// 			contactRight = b2 - f2 * ((0.0f - projB2) / (projA2 - projB2));
		// 		}
		// 		// clip rightwise
		// 		if (projA2 > 2 * len1 && projB2 < 2 * len1) {
		// 			contactLeft  = a2 + f2 * ((0.0f - projA2) / (projB2 - projA2));
		// 		} else if (projA2 < 2 * len1 && projB2 > 2 * len1) {
		// 			contactRight = b2 - f2 * ((0.0f - projB2) / (projA2 - projB2));
		// 		}
		// 		// clip forward
		// 		float depthLeft  = (contactLeft  - a1).dot(n),
		// 			  depthRight = (contactRight - a1).dot(n);
  //
		// 		if (depthLeft <= dist) {
		// 			manifold.AddPoint(contactLeft  + n * (0.5f * (cap1.radius - cap2.radius)), totalRadius - depthLeft);
		// 		}
		// 		if (depthRight <= dist) {
		// 			manifold.AddPoint(contactRight + n * (0.5f * (cap1.radius - cap2.radius)), totalRadius - depthRight);
		// 		}
  //
		// 		if (manifold.contactCount)
		// 			return manifold;
		// 	} else {
		// 		const fVector2 n = -sat.GetSepAxis();
		// 		fVector2 contactLeft = a1, contactRight = b1;
		// 		// clip leftwise
		// 		if        (projA1 < 0 && projB1 > 0) {
		// 			contactLeft  = a1 + f1 * ((0.0f - projA1) / (projB1 - projA1));
		// 		} else if (projA1 > 0 && projB1 < 0) {
		// 			contactRight = b1 - f1 * ((0.0f - projB1) / (projA1 - projB1));
		// 		}
		// 		// clip rightwise
		// 		if        (projA1 > 2 * len2 && projB1 < 2 * len2) {
		// 			contactLeft  = a1 + f1 * ((projA1 - 2 * len2) / (projB1 - projA1));
		// 		} else if (projA1 < 2 * len2 && projB1 > 2 * len2) {
		// 			contactRight = b1 - f1 * ((projB1 - 2 * len2) / (projA1 - projB1));
		// 		}
		// 		const float depthLeft  = (contactLeft  - a2).dot(n),
		// 					depthRight = (contactRight - a2).dot(n);
  //
		// 		if (depthLeft <= dist) {
		// 			manifold.AddPoint(contactLeft  + n * (0.5f * (cap2.radius - cap1.radius)), totalRadius - depthLeft);
		// 		}
		// 		if (depthRight <= dist) {
		// 			manifold.AddPoint(contactRight + n * (0.5f * (cap2.radius - cap1.radius)), totalRadius - depthRight);
		// 		}
  //
		// 		if (manifold.contactCount)
		// 			return manifold;
		// 	}
		// }
  //
		// // single point collision
	 //    {
		//     fVector2 normal = closest2 - closest1;
  //       	if (normal.lensq() > EPSILON) {
  //       		normal *= 1 / normal.len();
  //       	} else {
  //       		normal = fn1.perpend();
  //       	}
  //
  //       	const fVector2 c1 = closest1 + normal * cap1.radius,
		// 				   c2 = closest2 - normal * cap2.radius;
  //
  //       	manifold.seperatingNormal = normal;
  //       	manifold.AddPoint((c1 + c2) * 0.5f, totalRadius - dist);
	 //    }
  //   	return manifold;

        SeperatingAxisSolver sat = SeperatingAxisSolver::CheckCollisionFor(s1, xf1, s2, xf2);

        const fVector2 f1 = xf1.TransformDir(cap1.forward),
                       f2 = xf2.TransformDir(cap2.forward);

        const fVector2 off = xf1.position - xf2.position;
        const fVector2 tip1 =  off + f1, end1 =  off - f1,
                       tip2 = -off + f2, end2 = -off - f2;
        const fVector2 axis1 = tip1 - f2 * std::clamp(tip1.dot(f2) * cap2.invLenSq, -1.0f, 1.0f),
                       axis2 = end1 - f2 * std::clamp(end1.dot(f2) * cap2.invLenSq, -1.0f, 1.0f),
                       axis3 = tip2 - f1 * std::clamp(tip2.dot(f1) * cap1.invLenSq, -1.0f, 1.0f),
                       axis4 = end2 - f1 * std::clamp(end2.dot(f1) * cap1.invLenSq, -1.0f, 1.0f);

        sat.SetCheckFor(SeperatingAxisSolver::NEITHER);

        bool useSecondCapsule = false;

        sat.CheckAxis(axis1.norm());
        sat.CheckAxis(axis2.norm());
        useSecondCapsule |= sat.CheckAxis(axis3.norm());
        useSecondCapsule |= sat.CheckAxis(axis4.norm());

        if (!sat.Collides())
            return Manifold::None();

        return Manifold {
            .seperatingNormal = sat.GetSepAxis(),
            .contactPoint = {
                useSecondCapsule ?
                xf2.Transform(cap2.FurthestAlong(xf2.TransformInverseDir(-sat.GetSepAxis()))) :
                xf1.Transform(cap1.FurthestAlong(xf1.TransformInverseDir( sat.GetSepAxis())))
            },
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
            .contactPoint = { xf2.Transform(cap.FurthestAlong(xf2.TransformInverseDir(-sat.GetSepAxis()))) },
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
        constexpr auto TAG = [] (const u32 ccount, bool bdyn, bool tdyn) constexpr -> u32 {
            return ccount << 2 |
                   bdyn << 1 |
                   tdyn;
        };

        switch (TAG(manifold.contactCount, body.IsDynamic(), target.IsDynamic())) {
            case TAG(1, true,  true ): DynamicResolveFor<1, true,  true >(body, target, manifold); break;
            case TAG(1, true,  false): DynamicResolveFor<1, true,  false>(body, target, manifold); break;
            case TAG(1, false, true ): DynamicResolveFor<1, false, true >(body, target, manifold); break;
            case TAG(2, true,  true ): DynamicResolveFor<2, true,  true >(body, target, manifold); break;
            case TAG(2, true,  false): DynamicResolveFor<2, true,  false>(body, target, manifold); break;
            case TAG(2, false, true ): DynamicResolveFor<2, false, true >(body, target, manifold); break;
            default: Debug::QError$("bad collision response");
        }
    }

    template <u32 ContactCount, bool BDyn, bool TDyn>
    void DynamicResolveFor(Body& body, Body& target, const Manifold& manifold) {
        const fVector2& normal = manifold.seperatingNormal, tangent = normal.perpend();
        const float share = ContactCount == 2 ? 0.5f : 1.0f;
        // this makes contactCount known in compile time for loop unrolling

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

        constexpr float BODY_RESTITUION = 1.0f;
        constexpr float STATIC_FRICTION = 0.75f, DYNAMIC_FRICTION = 0.57f;

        fVector2 relBody    [ContactCount], relTarget[ContactCount];
        float    velocityMag[ContactCount], jn       [ContactCount];

        fVector2 totalImpulse = 0;
        [[maybe_unused]] float totalBodyAngMtm = 0;
        [[maybe_unused]] float totalTargetAngMtm = 0;
        // normal forces
        for (u32 i = 0; i < ContactCount; ++i) {
            const fVector2& contact = manifold.contactPoint[i];

            relBody  [i] = contact - body.position,
            relTarget[i] = contact - target.position;

            const fVector2 relVel = [&] {
                if constexpr (BDyn && TDyn) {
                    const fVector2 angularVelBody   = relBody  [i].perpend() * -body.angularVelocity,
                                   angularVelTarget = relTarget[i].perpend() * -target.angularVelocity;

                    return (target.velocity + angularVelTarget) -
                           (body  .velocity + angularVelBody);
                } else if constexpr (BDyn) {
                    return -body.velocity + relBody[i].perpend() * body.angularVelocity;
                } else /* targetDyn */ {
                    return target.velocity - relTarget[i].perpend() * target.angularVelocity;
                }
            } ();

            velocityMag[i] = relVel.dot(normal);
            if (velocityMag[i] > 0.0f) continue;

            const float denomN = [&] {
                if constexpr (BDyn && TDyn) {
                    const float torqueRadiusNBody   = relBody  [i].zcross(normal),
                                torqueRadiusNTarget = relTarget[i].zcross(normal);

                    return body.invMass + target.invMass +
                          (torqueRadiusNBody   * torqueRadiusNBody)   * body  .invInertia +
                          (torqueRadiusNTarget * torqueRadiusNTarget) * target.invInertia;
                } else if constexpr (BDyn) {
                    const float torqueRadius = relBody[i].zcross(normal);
                    return body.invMass + torqueRadius * torqueRadius * body.invInertia;
                } else /* targetDyn */ {
                    const float torqueRadius = relTarget[i].zcross(normal);
                    return target.invMass + torqueRadius * torqueRadius * target.invInertia;
                }
            } ();

            jn[i] = -(1.0f + BODY_RESTITUION) * velocityMag[i] * share / denomN;

            const fVector2 impulse = jn[i] * normal;

            totalImpulse      += impulse;
            if constexpr (BDyn) totalBodyAngMtm   -= relBody  [i].zcross(impulse);
            if constexpr (TDyn) totalTargetAngMtm += relTarget[i].zcross(impulse);
        }

        if constexpr (BDyn) {
            body.AddMomentum(-totalImpulse);
            body.AddAngularMomentum(totalBodyAngMtm);
        }
        if constexpr (TDyn) {
            target.AddMomentum(totalImpulse);
            target.AddAngularMomentum(totalTargetAngMtm);
        }

        totalImpulse = 0;
        totalBodyAngMtm = 0; totalTargetAngMtm = 0;
        // frictional forces (needs to be separated, order matters)
        for (u32 i = 0; i < ContactCount; ++i) {
            const fVector2 relVel = [&] {
                if constexpr (BDyn && TDyn) {
                    const fVector2 angularVelBody   = relBody  [i].perpend() * -body.angularVelocity,
                                   angularVelTarget = relTarget[i].perpend() * -target.angularVelocity;

                    return (target.velocity + angularVelTarget) -
                           (body  .velocity + angularVelBody);
                } else if constexpr (BDyn) {
                    return -body.velocity + relBody[i].perpend() * body.angularVelocity;
                } else /* targetDyn */ {
                    return target.velocity - relTarget[i].perpend() * target.angularVelocity;
                }
            } ();

            const float t = relVel.dot(tangent);

            if (std::abs(t) <= f32s::EPSILON) continue;

            const float denomT = [&] {
                if constexpr (BDyn && TDyn) {
                    const float torqueRadiusTBody   = relBody  [i].zcross(tangent),
                                torqueRadiusTTarget = relTarget[i].zcross(tangent);

                    return body.invMass + target.invMass +
                          (torqueRadiusTBody   * torqueRadiusTBody)   * body  .invInertia +
                          (torqueRadiusTTarget * torqueRadiusTTarget) * target.invInertia;
                } else if constexpr (BDyn) {
                    const float torqueRadius = relBody[i].zcross(tangent);
                    return body.invMass + torqueRadius * torqueRadius * body.invInertia;
                } else /* targetDyn */ {
                    const float torqueRadius = relTarget[i].zcross(tangent);
                    return target.invMass + torqueRadius * torqueRadius * target.invInertia;
                }
            } ();

            const float jt = -t * share / denomT;
            const fVector2 frictionImpulse = tangent * (
                std::abs(jt) <= std::abs(jn[i] * STATIC_FRICTION) ?
                jt : (jt < 0 ? -1.0f : 1.0f) * jn[i] * DYNAMIC_FRICTION
            ); // static/dynamic friction picking

            totalImpulse      += frictionImpulse;
            if constexpr (BDyn) totalBodyAngMtm   -= relBody  [i].zcross(frictionImpulse);
            if constexpr (TDyn) totalTargetAngMtm += relTarget[i].zcross(frictionImpulse);
        }

        if constexpr (BDyn) {
            body.AddMomentum(-totalImpulse);
            body.AddAngularMomentum(totalBodyAngMtm);
        }
        if constexpr (TDyn) {
            target.AddMomentum(totalImpulse);
            target.AddAngularMomentum(totalTargetAngMtm);
        }
    }

    template void DynamicResolveFor<1, true,  true >(Body& body, Body& target, const Manifold& manifold);
    template void DynamicResolveFor<1, true,  false>(Body& body, Body& target, const Manifold& manifold);
    template void DynamicResolveFor<1, false, true >(Body& body, Body& target, const Manifold& manifold);
    template void DynamicResolveFor<2, true,  true >(Body& body, Body& target, const Manifold& manifold);
    template void DynamicResolveFor<2, true,  false>(Body& body, Body& target, const Manifold& manifold);
    template void DynamicResolveFor<2, false, true >(Body& body, Body& target, const Manifold& manifold);
} // Physics2D