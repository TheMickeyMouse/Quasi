#pragma once
#include "Collider2D.h"

namespace Physics2D {
    class CircleCollider : public Collider {
    public:
        float radius = 0;

        CircleCollider(const Maths::fvec2& position, float mass, float radius) : Collider(position, mass), radius(radius) {}

        [[nodiscard]] OPENGL_API bool CollidesWith(const Collider& other) const override;

        [[nodiscard]] OPENGL_API bool CollideCircle(const CircleCollider& other) const;
        [[nodiscard]] OPENGL_API bool CollidePoint(const Maths::fvec2& other) const;

        OPENGL_API void StaticResolve(CircleCollider& other);
        OPENGL_API void DynamicResolve(CircleCollider& other);
    };
} // Physics2D
