#pragma once
#include "Shape2D.h"
#include "PhysicsTransform2D.h"

namespace Quasi::Physics2D {
    class Body;
}

namespace Quasi::Physics2D::Collision {
    constexpr u32 IndexPair(u32 x, u32 y) { return x * MAX + y; }

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
        [[nodiscard]] Event SwapIf(bool swap) const { return swap ? Swap() : *this; }
        [[nodiscard]] Math::fVector2 Seperator() const { return contactPointTarget - contactPointBase; }

        [[nodiscard]] bool Valid() const { return collides && contactPointTarget.distsq(contactPointBase) > Math::EPSILON * Math::EPSILON; }

        static const Event None;

        using CollisionCheckFunc = FnPtr<Event, const Shape&, const PhysicsTransform&, const Shape&, const PhysicsTransform&>;
    };
    inline const Event Event::None = {};

    Event CollideShapeDyn(const Shape& s1, const PhysicsTransform& xf1, const Shape& s2, const PhysicsTransform& xf2);

    template <IShape T, IShape U> requires (T::Index <= U::Index)
    Event CollideShape(const T& shape1, const PhysicsTransform& xf1, const U& shape2, const PhysicsTransform& xf2);
    template <IShape T, IShape U> requires (T::Index > U::Index)
    Event CollideShape(const T& shape1, const PhysicsTransform& xf1, const U& shape2, const PhysicsTransform& xf2) {
        return CollideShape(shape2, xf2, shape1, xf1).Swap();
    }

    template <IShape T, IShape U>
    Event EraseCollisionPtr(const Shape& s1, const PhysicsTransform& x1, const Shape& s2, const PhysicsTransform& x2) {
        return CollideShape<T, U>(dynamic_cast<const T&>(s1), x1, dynamic_cast<const U&>(s2), x2);
    }

    void StaticResolve (Body& body, Body& target, const Event& cEvent);
    void DynamicResolve(Body& body, Body& target, const Event& cEvent);
} // Physics2D::Collision
