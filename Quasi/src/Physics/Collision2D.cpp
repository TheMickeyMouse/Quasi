#include "Collision2D.h"

#include "Body2D.h"

namespace Quasi::Physics2D::Collision {
    const Array<Event::CollisionCheckFunc, IndexPair(MAX, 0)> Event::CollisionTable = []{
        Array<CollisionCheckFunc, IndexPair(MAX, 0)> array;
        array[IndexPair(CIRCLE,   CIRCLE)]   = Erase<Circle2Circle>();
        array[IndexPair(CIRCLE,   EDGE)]     = Erase<Circle2Edge>();
        array[IndexPair(EDGE,     EDGE)]     = Erase<Edge2Edge>();
        array[IndexPair(CIRCLE,   TRIANGLE)] = Erase<Circle2Triangle>();
        array[IndexPair(EDGE,     TRIANGLE)] = nullptr;
        array[IndexPair(TRIANGLE, TRIANGLE)] = nullptr;
        return array;
    }();

    Event Between(const Shape& s1, TransformRef t1, const Shape& s2, TransformRef t2) {
        const u32 y1 = s1.TypeIndex(), y2 = s2.TypeIndex();
        return y2 >= y1 ? Event::CollisionTable[IndexPair(y2, y1)](s2, t2, s1, t1) :
                          Event::CollisionTable[IndexPair(y1, y2)](s1, t1, s2, t2).Swap();
    }

    Event Circle2Circle(const CircleShape& c1, TransformRef t1, const CircleShape& c2, TransformRef t2) {
        const Math::fVector2 diff = t2.position - t1.position;
        return diff.lensq() < (c1.radius + c2.radius) * (c1.radius + c2.radius) ?
            Event { t1.position + diff.norm(c1.radius), t2.position - diff.norm(c2.radius) } :
            Event::None;
    }

    Event Circle2Edge(const CircleShape& c1, TransformRef t1, const EdgeShape& e2, TransformRef t2) {
        const Math::fVector2 closestPoint = Math::fLine2D { t2 * e2.start, t2 * e2.end }.NearestTo(t1.position);
        return Circle2Circle(c1, t1, CircleShape { e2.radius }, closestPoint);
    }

    Event Edge2Edge(const EdgeShape& e1, TransformRef t1, const EdgeShape& e2, TransformRef t2) {
        const Math::fVector2 e1s = t1 * e1.start, e1e = t1 * e1.end, e2s = t2 * e2.start, e2e = t2 * e2.end;
        const Math::fVector2 projection = { e1e.y - e1s.y, e1s.x - e1e.x };
        const float rStart = (e2s - e1s).dot(projection), rEnd = (e2e - e1s).dot(projection);

        Math::fVector2 point2;
        if (std::min(rStart, rEnd) < 0 && 0 < std::max(rStart, rEnd)) { // line intersects
            point2 = e2s.lerp(e2e, rStart / (rStart - rEnd));

            const float t = (point2 - e1s).dot(e1e - e1s);
            if (t < 0 || e1e.distsq(e1s) < t) {
                const Math::fVector2& point1 = t < 0 ? e1s : e1e;
                return Circle2Circle(
                    CircleShape { e1.radius }, point1,
                    CircleShape { e2.radius }, Math::fLine2D { e2s, e2e }.NearestTo(point1)
                );
            }
        } else point2 = std::abs(rStart) < std::abs(rEnd) ? e2s : e2e; // clamped

        const Math::fVector2 point1 = Math::fLine2D { e1s, e1e }.NearestTo(point2);

        return Circle2Circle(CircleShape { e1.radius }, point1,
                             CircleShape { e2.radius }, point2);
    }

    Event Circle2Triangle(const CircleShape& c1, TransformRef t1, const TriangleShape& r2, TransformRef t2) {
        Math::fTriangle2D tri { r2.a, r2.b, r2.c };
        for (const auto p : tri) {
            Math::fVector2 tp = t2 * p;
            if (t1.position.distsq(tp) < c1.radius) {
                return Event { t1.position + (tp - t1.position).norm(c1.radius), tp };
            }
        }
        if (tri.contains(t1.position)) return Event { t1.position, t2 * (tri.p1 + tri.p2 + tri.p3) / 3.0f };

        for (u32 i = 0; i < 3; ++i) {
            if (const Event e = Circle2Edge(c1, t1, EdgeShape { tri.point(i), tri.point(i == 2 ? 0 : i + 1), 0.0f }, t2))
                return e;
        }
        return Event::None;
    }

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