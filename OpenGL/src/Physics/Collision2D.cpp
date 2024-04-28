#include "Collision2D.h"

#include <algorithm>

#include "Body2D.h"

#include <typeindex>

namespace Physics2D::Collision {
    Event Between(const Shape& s1, TransformRef t1, const Shape& s2, TransformRef t2) {
        const std::type_index s1t = typeid(s1), s2t = typeid(s2);
        #define CASE(T1, T2, FN) \
        if (s1t == typeid(T1) && s2t == typeid(T2)) \
            return FN(dynamic_cast<const T1&>(s1), t1, dynamic_cast<const T2&>(s2), t2); \
        if (s1t == typeid(T2) && s2t == typeid(T1)) \
            return FN(dynamic_cast<const T1&>(s2), t2, dynamic_cast<const T2&>(s1), t1).Swap();

        CASE(CircleShape, CircleShape, Circle2Circle);
        CASE(CircleShape, EdgeShape,   Circle2Edge  );
        return Event::None;
    }

    Event Circle2Circle(const CircleShape& c1, TransformRef t1, const CircleShape& c2, TransformRef t2) {
        const Maths::fvec2 diff = t2.position - t1.position;
        return diff.lensq() < (c1.radius + c2.radius) * (c1.radius + c2.radius) ?
            Event { t1.position + diff.norm(c1.radius), t2.position - diff.norm(c2.radius) } :
            Event::None;
    }

    Event Circle2Edge(const CircleShape& c1, TransformRef t1, const EdgeShape& e2, TransformRef t2) {
        const Maths::fvec2 start = t2 * e2.start, end = t2 * e2.end,
                           toCircle = t1.position - start;
        const float t = toCircle.dot(end - start) / (end - start).lensq();
        const Maths::fvec2 closestPoint = start.lerp(end, std::clamp(t, 0.0f, 1.0f));
        return Circle2Circle(c1, t1, CircleShape { 0.0f }, closestPoint);
    }

    void StaticResolve(Body& body, Body& target, const Event& cEvent) {
        const Maths::fvec2 sep = cEvent.Seperator();
        const bool bodyDyn = body.type == BodyType::DYNAMIC, targetDyn = target.type == BodyType::DYNAMIC;
        body.position   += bodyDyn   ? sep / (float)(bodyDyn + targetDyn) : 0.0f;
        target.position -= targetDyn ? sep / (float)(bodyDyn + targetDyn) : 0.0f;
    }

    void DynamicResolve(Body& body, Body& target, const Event& cEvent) {
        const Maths::fvec2 collisionDir = cEvent.Seperator();
        const float mb = body.mass, mt = target.mass;
        const Maths::fvec2 p = (2 * collisionDir.dot(target.velocity - body.velocity)) / ((mb + mt) * collisionDir.lensq()) * collisionDir;
        using enum BodyType;
        const bool sb = body.type == STATIC, st = target.type == STATIC;
        body.velocity   += sb ? 0 : (mt + (st ? mb : 0)) * p;
        target.velocity -= st ? 0 : (mb + (sb ? mt : 0)) * p;
    }
} // Physics2D