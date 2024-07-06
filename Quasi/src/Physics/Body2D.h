#pragma once
#include "Collision2D.h"

#include "Shape2D.h"
#include "Transform2D.h"
#include "Vector.h"

namespace Quasi::Physics2D {
    enum class BodyType {
        STATIC,
        DYNAMIC,
        KINEMATIC,
        NUM,
        NONE = -1
    };

    class World;

    class Body {
    public:
        Math::fVector2 position, velocity, acceleration;
        float mass = 1.0f;
        BodyType type = BodyType::NONE;
        bool enabled = true;
    private:
        Ref<World> world = nullptr;
    public:
        Ref<Shape> shape; // heap managed

        Body(const Math::fVector2& p, float m, BodyType type, Ref<World> world, Ref<Shape> shape)
            : position(p), mass(m), type(type), world(world), shape(shape) {}

        void AddVelocity(const Math::fVector2& vel) { velocity += vel; }
        void Stop() { velocity = 0; }

        [[nodiscard]] Collision::Event CollidesWith(const Body& target) const;
        [[nodiscard]] Collision::Event CollidesWith(const Shape& target, const Transform& t) const;
        [[nodiscard]] Transform GetTransform() const;

        void Update(float dt);

        [[nodiscard]] bool IsStatic()  const { return type == BodyType::STATIC; }
        [[nodiscard]] bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }

        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;

        friend class World;
        friend void Collision::StaticResolve (Body&, Body&, const Collision::Event&);
        friend void Collision::DynamicResolve(Body&, Body&, const Collision::Event&);
    };

    struct BodyCreateOptions {
        Math::fVector2 position {};
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
    };
} // Physics2D
