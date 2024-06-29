#pragma once
#include "Shape2D.h"
#include "Transform2D.h"

namespace Quasi::Physics2D {
    class Body;
}

namespace Quasi::Physics2D::Collision {
    using TransformRef = const Transform&;

    struct Event {
        bool collides;
        Math::fVector2 contactPointBase, contactPointTarget;

    private:
        Event() : collides(false) {}
    public:
        Event(const Math::fVector2& cpBase, const Math::fVector2& cpTarget)
            : collides(true), contactPointBase(cpBase), contactPointTarget(cpTarget) {}

        [[nodiscard]] operator bool() const { return collides; }
        [[nodiscard]] Event Swap() const { return collides ? Event { contactPointTarget, contactPointBase } : None; }
        [[nodiscard]] Math::fVector2 Seperator() const { return contactPointTarget - contactPointBase; }

        static const Event None;
    };
    inline const Event Event::None = {};

    Event Between(const Shape& s1, TransformRef t1, const Shape& s2, TransformRef t2);

    Event Circle2Circle(const CircleShape& c1, TransformRef t1, const CircleShape& c2, TransformRef t2);
    Event Circle2Edge  (const CircleShape& c1, TransformRef t1, const EdgeShape&   e2, TransformRef t2);

    void StaticResolve (Body& body, Body& target, const Event& cEvent);
    void DynamicResolve(Body& body, Body& target, const Event& cEvent);
} // Physics2D::Collision
