#pragma once
#include "Shape2D.h"
#include "Transform2D.h"

namespace Quasi::Physics2D {
    class Body;
}

namespace Quasi::Physics2D::Collision {
    using TransformRef = const Transform&;

    constexpr u32 IndexPair(u32 x, u32 y) { return y > x ? y * (y + 1) / 2 + x : x * (x + 1) / 2 + y; }

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

        using CollisionCheckFunc = FnPtr<Event, const Shape&, TransformRef, const Shape&, TransformRef>;
        static const Array<CollisionCheckFunc, IndexPair(MAX, 0)> CollisionTable;
        template <auto F> static CollisionCheckFunc Erase() {
            return +[](const Shape& s1, TransformRef t1, const Shape& s2, TransformRef t2) {
                return F(
                    dynamic_cast<std::tuple_element_t<0, ArgumentsOf<decltype(F)>>>(s2), t2,
                    dynamic_cast<std::tuple_element_t<2, ArgumentsOf<decltype(F)>>>(s1), t1);
            };
        }
    };
    inline const Event Event::None = {};

    Event Between(const Shape& s1, TransformRef t1, const Shape& s2, TransformRef t2);

    Event Circle2Circle  (const CircleShape&, TransformRef, const CircleShape&  , TransformRef);
    Event Circle2Edge    (const CircleShape&, TransformRef, const EdgeShape&    , TransformRef);
    Event Edge2Edge      (const EdgeShape&  , TransformRef, const EdgeShape&    , TransformRef);
    Event Circle2Triangle(const CircleShape&, TransformRef, const TriangleShape&, TransformRef);

    void StaticResolve (Body& body, Body& target, const Event& cEvent);
    void DynamicResolve(Body& body, Body& target, const Event& cEvent);
} // Physics2D::Collision
