#include "Collision2D.h"

#include "Body2D.h"
#include "Utils/Macros.h"

namespace Quasi::Physics2D::Collision {

#define SHAPES(M, ...)  M(CircleShape, __VA_ARGS__) M(EdgeShape, __VA_ARGS__) M(TriangleShape, __VA_ARGS__)
#define PAIR_HELPER(S, M, ...) Q_DEFER(Q_CAT)(SHAP, ES) (M, S, __VA_ARGS__)
#define SHAPE_PAIRS(M, ...) Q_UNARY(SHAPES(PAIR_HELPER, M, __VA_ARGS__))

    Event CollideShapeDyn(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2) {
#define ARRAY_SET_EACH(Y, X) EraseCollisionPtr<X, Y>,
#define ARRAY_SET(X, _) { Q_DEFER(Q_CAT)(SHAPE, S)(ARRAY_SET_EACH, X) },
        constexpr Event::CollisionCheckFunc COLLISION_LOOKUP_TABLE[MAX][MAX] = {
            Q_UNARY(SHAPES(ARRAY_SET))
        };

        const u32 y1 = s1.TypeIndex(), y2 = s2.TypeIndex();
        return COLLISION_LOOKUP_TABLE[y1][y2](s1, xf1, s2, xf2);
    }

#define COLLIDE_SHAPE_DEFINE(S1, S2) \
    template <> \
    Event CollideShape<S1, S2>(const S1& s1, const PhysicsTransform& xf1, const S2& s2, const PhysicsTransform& xf2)

    COLLIDE_SHAPE_DEFINE(CircleShape, CircleShape) {
        const Math::fVector2 diff = xf2.position - xf1.position;
        return diff.lensq() < (s1.radius + s2.radius) * (s1.radius + s2.radius) ?
            Event { xf1.position + diff.norm(s1.radius), xf2.position - diff.norm(s2.radius) } :
            Event::None;
    }

    COLLIDE_SHAPE_DEFINE(CircleShape, EdgeShape) {
        const Math::fVector2 closestPoint = Math::fLine2D { xf2 * s2.start, xf2 * s2.end }.nearest_to(xf1.position);
        return CollideShape(s1, xf1, CircleShape { s2.radius }, closestPoint);
    }

    COLLIDE_SHAPE_DEFINE(EdgeShape, EdgeShape) {
#if 0
        const Math::fVector2 e1s = xf1 * s1.start, e1e = xf1 * s1.end, e2s = xf2 * s2.start, e2e = xf2 * s2.end;
        const Math::fVector2 projection = { e1e.y - e1s.y, e1s.x - e1e.x };
        const float rStart = (e2s - e1s).dot(projection), rEnd = (e2e - e1s).dot(projection);

        Math::fVector2 point2;
        if (std::min(rStart, rEnd) < 0 && 0 < std::max(rStart, rEnd)) { // line intersects
            point2 = e2s.lerp(e2e, rStart / (rStart - rEnd));

            const float t = (point2 - e1s).dot(e1e - e1s);
            if (t < 0 || e1e.distsq(e1s) < t) {
                const Math::fVector2& point1 = t < 0 ? e1s : e1e;
                return CollideShape(
                    CircleShape { s1.radius }, point1,
                    CircleShape { s2.radius }, Math::fLine2D { e2s, e2e }.nearest_to(point1)
                );
            }
        } else point2 = std::abs(rStart) < std::abs(rEnd) ? e2s : e2e; // clamped

        const Math::fVector2 point1 = Math::fLine2D { e1s, e1e }.nearest_to(point2);

        return CollideShape(CircleShape { s1.radius }, point1,
                            CircleShape { s2.radius }, Math::fLine2D { e2s, e2e }.nearest_to(point1));
#endif
        const auto [closest1, closest2] =
            Math::fLine2D { xf1 * s1.start, xf1 * s1.end }.nearest_between({ xf2 * s2.start, xf2 * s2.end });
        return CollideShape(CircleShape { s1.radius }, closest1, CircleShape { s2.radius }, closest2);
    }

    COLLIDE_SHAPE_DEFINE(CircleShape, TriangleShape) {
        Math::fTriangle2D tri { xf2 * s2.a, xf2 * s2.b, xf2 * s2.c };
        const u32 collisionPoints = (xf1.position.in_range(tri.p1, s1.radius) << 0) |
                                    (xf1.position.in_range(tri.p2, s1.radius) << 1) |
                                    (xf1.position.in_range(tri.p3, s1.radius) << 2);
        if ((0b00010110 >> collisionPoints) & 1) { // single point
            const Math::fVector2& point = tri.point((0x21100 >> (collisionPoints * 4)) & 0xF);
            return Event { xf1.position + (point - xf1.position).norm(tri.contains(xf1.position) ? -s1.radius : s1.radius), point };
        }
        if ((0b01101000 >> collisionPoints) & 1) { // 2 points
            const Math::fLine2D line = tri.line((0x02211 >> ((7 ^ collisionPoints) * 4)) & 0xF);
            const Math::fVector2 point = line.nearest_to(xf1.position);
            return Event { xf1.position + (point - xf1.position).norm(tri.contains(xf1.position) ? -s1.radius : s1.radius), point };
        }
        if (collisionPoints == 7) { // all points
            const float distances[3] = {
                xf1.position.distsq(tri.p1),
                xf1.position.distsq(tri.p2),
                xf1.position.distsq(tri.p3),
            };
            const Math::fVector2& point = tri.point(std::min_element(distances, distances + 3) - distances);
            return Event { xf1.position + (point - xf1.position).norm(tri.contains(xf1.position) ? -s1.radius : s1.radius), point };
        }

        const Math::fVector2 bary = tri.barycentric(xf1.position);
        const u32 cat = Math::fTriangle2D::categorize_bary(bary);
        const Math::fLine2D closestLine = tri.line(cat);
        const Math::fVector2 closest = closestLine.nearest_to(xf1.position);
        const bool sideCenter = tri.is_clockwise(); // if its clockwise then center is on the left
        const bool sideCircle = closestLine.leftside(xf1.position);
        if (sideCenter == sideCircle || closest.distsq(xf1.position) <= s1.radius * s1.radius) {
            return Event { xf1.position + (closest - xf1.position).norm(sideCenter == sideCircle ? -s1.radius : s1.radius), closest };
        }
        return Event::None;
    }

    COLLIDE_SHAPE_DEFINE(EdgeShape, TriangleShape) {
        const Math::fTriangle2D tri { xf2 * s2.a, xf2 * s2.b, xf2 * s2.c };
        const Math::fLine2D line { xf1 * s1.start, xf1 * s1.end };

        if (tri.contains(line.start) && tri.contains(line.end)) {
            const Math::fVector2 mid = line.lerp(0.5f);
            const Math::fVector2 directPoint = tri.line(tri.categorize(mid)).nearest_to(mid);
            return Event { mid + (line.leftside(directPoint) ? -1.0f : 1.0f) * line.forward().perpend().norm(s1.radius), directPoint };
        }

        float closestDistance = INFINITY; // stored as sq(x)
        Math::fVector2 triClosest, lineClosest;
        bool useDirectPush = false;
        for (u32 i = 0; i < 3; ++i) {
            const auto& p = tri.point(i);
            const Math::fVector2 closestPoint = line.nearest_to(p);
            if (!p.in_range(closestPoint, s1.radius)) {
                const Math::fLine2D triLine = tri.line(i);
                const auto [lineU, triU] = line.intersect_solutions(triLine);
                if (0 <= triU && triU <= 1) {
                    const Math::fVector2 linePoint = line.lerp(lineU);
                    if (0 <= lineU && lineU <= 1) {
                        if (const float dist = (0.5f - lineU) * (0.5f - lineU) * line.lensq(); dist < closestDistance) {
                            closestDistance = dist;
                            triClosest = linePoint;
                            lineClosest = lineU < 0.5f ? line.start - line.forward().norm(s1.radius) : line.end + line.forward().norm(s1.radius);
                            useDirectPush = true;
                        }
                        continue;
                    }
                }
                const Math::fVector2& side = lineU < 0 ? line.start : line.end;
                const Math::fVector2 clampedTriPoint = triLine.nearest_to(side);
                const float dist = side.distsq(clampedTriPoint);
                if (dist < s1.radius * s1.radius && dist < closestDistance) {
                    closestDistance = dist;
                    triClosest = clampedTriPoint;
                    lineClosest = side;
                }
            } else if (const float dist = closestPoint.distsq(p); dist < closestDistance) {
                closestDistance = dist * dist;
                triClosest = p;
                lineClosest = closestPoint;
            }
        }
        if (closestDistance != INFINITY) {
            if (useDirectPush) {
                return Event { lineClosest, triClosest };
            }
            return Event { lineClosest + (triClosest - lineClosest).norm(s1.radius), triClosest };
        }

        return Event::None;
#if 0

        const u32 sideofPoint =
            (line.leftside(tri.p1) << 0) |
            (line.leftside(tri.p2) << 1) |
            (line.leftside(tri.p3) << 2);
        const u32 majoritySide = (0x32212110 >> (sideofPoint * 4)) & 0xF;

        if (majoritySide == 0 || majoritySide == 3) {
            float closestDistance = INFINITY;
            Math::fVector2 triClosestPoint;
            Math::fVector2 lineClosestPoint;
            for (u32 i = 0; i < 3; ++i) {
                const auto& p = tri.point(i);
                const float nearestU = (p - line.start).dot(line.forward()) / line.lensq();
                if (nearestU < 0 || nearestU > 1) {
                    const bool side = nearestU > 1;
                    const Math::fVector2& nearSide = side ? line.end : line.start;
                    const Math::fLine2D line1 = tri.line(i), line2 = tri.line((i + 2) % 3);
                    const float distLine1 = line1.nearest_to(nearSide).distsq(nearSide), distLine2 = line2.nearest_to(nearSide).distsq(nearSide);
                    const float& distMin = std::min(distLine1, distLine2);
                    if (distMin > s1.radius || distMin > closestDistance)
                        continue;
                    closestDistance = distMin;
                    triClosestPoint = (&distMin == &distLine1 ? line1 : line2).nearest_to(nearSide);
                    lineClosestPoint = nearSide;
                    continue;
                }
                const Math::fVector2 closestPoint = line.lerp(nearestU);
                if (const float dist = closestPoint.distsq(p); dist >= s1.radius * s1.radius)
                    continue;
                else if (dist < closestDistance) {
                    closestDistance = dist;
                    triClosestPoint = p;
                    lineClosestPoint = closestPoint;
                }
            }
            if (closestDistance != INFINITY)
                return Event { lineClosestPoint + (triClosestPoint - lineClosestPoint).norm(s1.radius), triClosestPoint };
            else return Event::None;
        }

        const bool majorityLeft = majoritySide == 2;
        const u32 uniqueSideI = (0x21100 >> ((majorityLeft ? 7 ^ sideofPoint : sideofPoint) * 4)) & 0xF;
        const Math::fVector2& uniquePoint = tri.point(uniqueSideI);
        const float nearestU = (uniquePoint - line.start).dot(line.forward()) / line.lensq();

        if (nearestU < 0 || nearestU > 1) {
            const bool side = nearestU > 1;
            const Math::fVector2& nearSide = side ? line.end : line.start;
            const Math::fLine2D line1 = tri.line(uniqueSideI), line2 = tri.line((uniqueSideI + 2) % 3);
            const float distLine1 = line1.nearest_to(nearSide).distsq(nearSide), distLine2 = line2.nearest_to(nearSide).distsq(nearSide);
            const float& distMin = std::min(distLine1, distLine2);
            if (distMin > s1.radius * s1.radius)
                return Event::None;
            const Math::fVector2 closerPoint = (&distMin == &distLine1 ? line1 : line2).nearest_to(nearSide);
            return Event { nearSide + (closerPoint - nearSide).norm(s1.radius), closerPoint };
        }
        const Math::fVector2 nearest = line.lerp(nearestU);
        return Event { nearest - (uniquePoint - nearest).norm(s1.radius), uniquePoint};
#endif
    }

    COLLIDE_SHAPE_DEFINE(TriangleShape, TriangleShape) {
#if 0
        const Math::fTriangle2D tri1 = { xf1 * s1.a, xf1 * s1.b, xf1 * s1.c },
                                tri2 = { xf2 * s2.a, xf2 * s2.b, xf2 * s2.c };
        u8 pointInTriangle = 0; // bitmask of points of tri2 in tri1
        Math::fVector3 baries[3];
        for (u32 i = 0; i < 3; ++i) {
            const auto& p = tri2.point(i);
            const Math::fVector3 bary = tri1.barycentric(p);
            const float uv = bary.x + bary.y;
            if (0 < uv && uv < 1 /* valid bary */) {
                pointInTriangle |= 1 << i;
                baries[i] = bary;
            }
        }

        switch (pointInTriangle) {
            case 1: case 2: case 4: { // 1 point overlap
                //              43210
                const u8 i = (0x2F10F >> pointInTriangle * 4) & 0xF, // map 421 -> 210, get only point thats collided
                         j = tri1.categorize_bary(baries[i]);
                return Event {
                    tri1.line(j).nearest_to(tri2.point(i)),
                    tri2.point(i)
                };
            }
            case 3: case 5: case 6: { // 2 point (edge) overlap
                //               6543210
                const u8 xi = (0x01F2FFF >> pointInTriangle * 4) & 0xF, // map 356 -> 012, get point thats not collided
                         xj = tri1.categorize(tri2.point(xi));

                //                          *6   *5    4   *3    2    1    0
                const u8 points1and2 = (0b1001'0010'0000'0100'0000'0000'0000 >> pointInTriangle * 4) & 0xF;
                // get the 2 points that do collide, packed each 2 bit indices, total 4 bits

                const Math::fLine2D line = tri1.line(xj);
                const Math::fVector2 forward = line.forward().norm().perpend();
                const float fdistL = forward.dot(tri2.point(points1and2 &  0x3)),
                            fdistR = forward.dot(tri2.point(points1and2 >> 2));
                const Math::fVector2& further =
                    std::abs(fdistL) > std::abs(fdistR) ? tri2.point(points1and2 & 0x3) : tri2.point(points1and2 >> 2);
                return Event { line.nearest_to(further), further };
            }
            case 7: {
                const Math::fVector2 center1 = tri1.center(), center2 = tri2.center();
                return Event { tri1.project(center2), tri2.project(center1) };
            }
            default:;
        }

        const Math::fVector2 center1 = tri1.center();
        const float distps[3] = {
            tri2.line(0).dist_to(center1),
            tri2.line(1).dist_to(center1),
            tri2.line(2).dist_to(center1)
        };
        const u8 closestI = std::min_element(distps, distps + 3) - distps;
        u8 sidesOfPoint = 0; // bitmask of points of tri1, on right or left side of tri2.edge[closestI]
        const Math::fLine2D closeLine = tri2.line(closestI);
        for (u32 i = 0; i < 3; ++i) {
            const auto& p = tri1.point(i);
            sidesOfPoint |= closeLine.leftside(p) ? 1 << i : 0;
        }
        //                       76543210
        const u8 uniqueSide = (0xF012210F >> sidesOfPoint * 4) & 0xF;
        if (uniqueSide == 0xF) return Event::None;
        return Event { tri1.point(uniqueSide), closeLine.nearest_to(center1) };
#endif
        const Math::fTriangle2D triA = { xf1 * s1.a, xf1 * s1.b, xf1 * s1.c },
                                triB = { xf2 * s2.a, xf2 * s2.b, xf2 * s2.c };
        // bitmask of point overlaps
        u8 pointAinB = 0, pointBinA = 0;
        Math::fVector3 baryCoords[2][3];
        for (u32 i = 0; i < 3; ++i) {
            baryCoords[0][i] = triB.barycentric(triA.point(i));
            pointAinB |= Math::fTriangle2D::is_valid_bary(baryCoords[0][i]) << i;
            baryCoords[1][i] = triA.barycentric(triB.point(i));
            pointBinA |= Math::fTriangle2D::is_valid_bary(baryCoords[1][i]) << i;
        }

        // cases: T1+T2
        // 0+0*3     (none)     -> no collision
        // 0+0*2+1   (shot)     -> push line away
        // 0+0*1+1+1 (star)     -> try all 3 sides to push
        // 0+1       (cut)      -> push 1 point away
        // 0+2       (sprout)   -> push furthest point away
        // 0+3       (superset) -> push furthest point away
        // 1+1       (bowtie)   -> push both points away from eachother
        // 1+2       (spike)    -> push furthest point away

        constexpr u32 LOG_POWERS_OF_2 = 0b10'01'01'00'00; // usage: bitshift 2x to get log
        constexpr auto LOG = [] (const u8 x) -> u8 { return (LOG_POWERS_OF_2 >> (x * 2)) & 3; };
        //                         7 6 5 4 3 2 1 0, get number of bits set
        constexpr u32 BITS_SET = 0x3'2'2'1'2'1'1'0;
        const u8 bitsAB = (BITS_SET >> (pointAinB * 4)) & 0xF, bitsBA = (BITS_SET >> (pointBinA * 4)) & 0xF;

        switch (bitsAB + bitsBA) {
            case 0: { // case none | shot | star
                // a is being categorized by b
                const u32 category[3] = { Math::fTriangle2D::categorize_bary(baryCoords[0][0]),
                                          Math::fTriangle2D::categorize_bary(baryCoords[0][1]),
                                          Math::fTriangle2D::categorize_bary(baryCoords[0][2]) };
                if (category[0] == category[1] && category[1] == category[2]) // no collision
                    return Event::None;

                const u32 uniqueCategory = category[0] ^ category[1] ^ category[2]; // duplicates will cancel out
                if (uniqueCategory == 3) { // star
                    const Math::fVector2 baseCenter = triB.center();
                    Math::fVector2 closestPoint = 0;
                    float closestDistance = INFINITY;
                    for (u32 i = 0; i < 3; ++i) {
                        const Math::fVector2 point = triA.line(i).nearest_to(baseCenter);
                        if (const float dist = point.distsq(baseCenter); closestDistance > dist) {
                            closestPoint = point;
                            closestDistance = dist;
                        }
                    }
                    return Event { closestPoint, triB.project(triA.center()) };
                }

                const u32 uniqueI = /* 0 * (c == uc) */ + 1 * (category[1] == uniqueCategory) + 2 * (category[2] == uniqueCategory);
                const Math::fLine2D candidate1 = triA.line(uniqueI), candidate2 = triA.line((uniqueI + 2) % 3);

                //                               _222'_111'_000
                //                               _210'_210'_210
                constexpr u64 DIRECTED_POINT = 0xFF20'F2F1'F01F;
                const u32 directI = DIRECTED_POINT >> (16 * category[uniqueI] + 4 * category[!uniqueI]) & 0xF;
                const Math::fVector2& directPoint  = triB.point(directI);
                const Math::fLine2D& bestCandidate = candidate1.dist_to(directPoint) > candidate2.dist_to(directPoint) ? candidate1 : candidate2;

                const Option intersect = bestCandidate.intersection({ 0.5 * (triB.point((directI + 1) % 3) + triB.point((directI + 1) % 3)), directPoint });
                return intersect ? Event { *intersect, directPoint } : Event::None;
            }
            case 1: {
                // case cut
                const bool cutterIsB = pointBinA;
                const Math::fTriangle2D& triCutter = cutterIsB ? triB : triA, triCuttee = cutterIsB ? triA : triB;
                const u32 cutPointI = LOG(pointAinB | pointBinA);

                const Math::fVector2& cutPoint = triCutter.point(cutPointI);
                const u8 section1 = Math::fTriangle2D::categorize_bary(baryCoords[cutterIsB][!cutPointI]),
                         section2 = Math::fTriangle2D::categorize_bary(baryCoords[cutterIsB][3 ^ !cutPointI ^ cutPointI]),
                         optimalSection = section1 == section2 || (triCuttee.line(section1).dist_to(cutPoint) < triCuttee.line(section2).dist_to(cutPoint))
                                        ? section1 :  section2;
                return Event { cutPoint, triCuttee.line(optimalSection).nearest_to(cutPoint) }.SwapIf(cutterIsB);
            }
            case 2: { // case sprout | bowtie
                if (bitsAB == 1) { // bowtie
                    const u8 cutAI = LOG(pointAinB), cutBI = LOG(pointBinA);

                    const Math::fVector2 upA = triA.line((cutAI + 1) % 3).forward().perpend(),
                                         upB = triB.line((cutBI + 1) % 3).forward().perpend();
                    const bool clockA = triA.is_clockwise(), clockB = triB.is_clockwise();
                    // if is clockwise, upA starting at cutA will point outwards instead of inwards
                    const u8 sideofA = cutAI - Math::fLine2D { 0, upB }.leftside(triA.point(cutAI)) == clockA,
                             sideofB = cutBI - Math::fLine2D { 0, upA }.leftside(triB.point(cutBI)) == clockB;

                    // calcuate intersection, p1 = 0, p2 = up, p3 = point(side), p4 = point(side + 1)
                    const Math::fVector2& sideAstart = triB.point(sideofA), &sideAend = triB.cyclic_point(sideofA + 1);
                    const float projectedAu = (upA.x *  sideAstart.y               - upA.y *  sideAstart.x) /
                                              (upA.x * (sideAstart.y - sideAend.y) - upA.y * (sideAstart.x - sideAend.x));
                    const Math::fVector2& sideBstart = triB.point(sideofB), &sideBend = triB.cyclic_point(sideofB + 1);
                    const float projectedBu = (upB.x *  sideBstart.y               - upB.y *  sideBstart.x) /
                                              (upB.x * (sideBstart.y - sideBend.y) - upB.y * (sideBstart.x - sideBend.x));
                    const Math::fVector2 projectedA = sideAstart + (sideAend - sideAstart) * projectedAu,
                                         projectedB = sideBstart + (sideBend - sideBstart) * projectedBu;

                    if (triA.point(cutAI).distsq(projectedA) > triB.point(cutBI).distsq(projectedB)) // pushing a is better
                        return Event { triA.point(cutAI), projectedA };
                    else
                        return Event { projectedB, triB.point(cutBI) };
                } else { // sprout
                    const bool sproutIsB = pointBinA;
                    const u8 sproutTipI   = LOG(7 ^ (pointAinB | pointBinA)),
                             sproutBaseI1 = !sproutTipI,
                             sproutBaseI2 = 3 ^ sproutTipI ^ sproutBaseI1;

                    const Math::fTriangle2D& triSprout = sproutIsB ? triB : triA, &triTrunk = sproutIsB ? triA : triB;

                    const u8 pushSide = Math::fTriangle2D::categorize_bary(baryCoords[sproutIsB][sproutTipI]);
                    const Math::fLine2D  baseline    = triTrunk.line(pushSide);
                    const Math::fVector2 nearSprout1 = baseline.nearest_to(triSprout.point(sproutBaseI1)),
                                         nearSprout2 = baseline.nearest_to(triSprout.point(sproutBaseI2));
                    if (triSprout.point(sproutBaseI1).distsq(nearSprout1) > triSprout.point(sproutBaseI2).distsq(nearSprout2)) // pushing sprout1 is better
                        return Event { triSprout.point(sproutBaseI1), nearSprout1 }.SwapIf(sproutIsB);
                    else
                        return Event { triSprout.point(sproutBaseI2), nearSprout2 }.SwapIf(sproutIsB);
                }
            }
            case 3: { // case superset | spike
                if (bitsAB && bitsBA) { // spike
                    const bool tipIsB = bitsBA == 2;
                    const Math::fTriangle2D& triTip = tipIsB ? triB : triA, &triBase = tipIsB ? triA : triB;
                    const u8 baseCutI = LOG(tipIsB ? pointAinB : pointBinA);

                    const Math::fVector2& baseCut = triBase.point(baseCutI);
                    return Event { triTip.line((LOG(7 ^ (tipIsB ? pointBinA : pointAinB)) + 1) % 3).nearest_to(baseCut), baseCut }.SwapIf(tipIsB);
                } else { // superset
                    const bool superIsB = pointAinB;
                    const Math::fTriangle2D& triSuper = superIsB ? triB : triA, &triSub = superIsB ? triA : triB;

                    const Math::fVector2 superCenter = triSuper.center();
                    float closestDistance = INFINITY;
                    Math::fVector2 closestPoint = 0;
                    for (u32 i = 0; i < 3; ++i) {
                        const Math::fVector2 point = triSub.line(i).nearest_to(superCenter);
                        if (const float dist = point.distsq(superCenter); closestDistance > dist) {
                            closestPoint = point;
                            closestDistance = dist;
                        }
                    }
                    return Event { triSuper.project(closestPoint), closestPoint }.SwapIf(superIsB);
                }
            }
            default: return Event::None;
        }
    }

#define FN_INSTANTIATE(X, Y, _Z) template Event CollideShape<X, Y>(const X&, const PhysicsTransform&, const Y&, const PhysicsTransform&);
    SHAPE_PAIRS(FN_INSTANTIATE)
#undef FN_INSTANTIATE

    void StaticResolve(Body& body, Body& target, const Event& cEvent) {
        const Math::fVector2 sep = cEvent.Seperator();
        const bool bodyDyn = body.type == BodyType::DYNAMIC, targetDyn = target.type == BodyType::DYNAMIC;
        body.position   += bodyDyn   ? sep / (float)(bodyDyn + targetDyn) : 0.0f;
        target.position -= targetDyn ? sep / (float)(bodyDyn + targetDyn) : 0.0f;
    }

    void DynamicResolve(Body& body, Body& target, const Event& cEvent) {
        const Math::fVector2 collisionDir = cEvent.Seperator();
        const float mb = body.mass, mt = target.mass;
        const Math::fVector2 p = (2 * collisionDir.dot(target.velocity - body.velocity)) / ((mb + mt) * collisionDir.lensq()) * collisionDir;
        using enum BodyType;
        const bool sb = body.type == STATIC, st = target.type == STATIC;
        body.velocity   += sb ? 0 : (mt + (st ? mb : 0)) * p;
        target.velocity -= st ? 0 : (mb + (sb ? mt : 0)) * p;
    }
} // Physics2D