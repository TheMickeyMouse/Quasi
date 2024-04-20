#include "Collision2D.h"
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
        // equation for line = start + (end - start) * t; 0 <= t <= 1
        // given x*x + y*y = radius*radius, x and y is offseted by transform2
        // (sx * (1 - t) + ex * t)^2 + (sy * (1 - t) + ey * t)^2 = radius^2
        // sx^2 * (1 - 2t + t^2) + ex^2 * t^2 + 2*sx*ex*t - 2*sx*ex*t^2 +
        // sy^2 * (1 - 2t + t^2) + ey^2 * t^2 + 2*sy*ey*t - 2*sy*ey*t^2 = r^2
        // (sx^2 + sy^2 + ex^2 + ey^2 - 2*sx*ex - 2*sy*ey) * t^2 +
        // (2*sx*ex + 2*sy*ey - 2 * sx^2 * sy^2) * t +
        // (sx^2 * sy^2 - r^2) = 0
        const Maths::fvec2 start = t2 * e2.start - t1.position, end = t2 * e2.end - t1.position;
        const float a = start.lensq() + end.lensq() - 2 * start.dot(end),
                    b = 2 * start.dot(end) - 2 * start.lensq(),
                    c = start.lensq() - c1.radius * c1.radius,
                    p = b / a * 0.5f,
                    q = std::sqrtf(b * b - 4 * a * c) / a * 0.5f,
                    solution1 = p - q,
                    solution2 = p + q;
        const bool i1 = 0 <= solution1 && solution1 <= 1, i2 = 0 <= solution2 && solution2 <= 1;
        if (i1 ^ i2) {
            const float& outSolution = i1 ? solution2 : solution1;
            const Maths::fvec2& linePoint = outSolution < 0 ? start : end;
            return Event { linePoint.norm(c1.radius) + t1.position, linePoint + t1.position };
        }
        const Maths::fvec2 linePoint = start.lerp(end, p);
        return linePoint.lensq() <= c1.radius * c1.radius ?
            Event { linePoint.norm(c1.radius) + t1.position, linePoint + t1.position } :
            Event::None;
    }

    void StaticResolve(Body& body, Body& target, const Event& cEvent) {
        const Maths::fvec2 sep = cEvent.Seperator() * 0.5f;
        body.position += sep;
        target.position -= sep;
    }

    void DynamicResolve(Body& body, Body& target, const Event& cEvent) {
        const Maths::fvec2 collisionDir = cEvent.Seperator().norm();
        const float p = 2 * (target.velocity.dot(collisionDir) - body.velocity.dot(collisionDir)) / (body.mass + target.mass);
        body.velocity += p * target.mass * collisionDir;
        target.velocity -= p * body.mass * collisionDir;
    }
} // Physics2D