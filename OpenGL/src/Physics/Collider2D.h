#pragma once
#include "core.h"
#include "Vector.h"

namespace Physics2D {
    class Collider {
    public:
        Maths::fvec2 position, velocity, acceleration;
        float mass = 1.0f;

        Collider(const Maths::fvec2& position, float mass = 1.0f) : position(position), mass(mass) {}
        virtual ~Collider() = default;

        OPENGL_API void Update(float dt);

        virtual bool CollidesWith(const Collider& other) const = 0;
    };
}
