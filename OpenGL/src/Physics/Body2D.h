#pragma once
#include "Collision2D.h"
#include "core.h"
#include "Shape2D.h"
#include "Transform2D.h"
#include "Vector.h"

namespace Physics2D {
    enum class BodyType {
        STATIC,
        DYNAMIC,
        KINEMATIC,
        NUM,
        NONE = -1
    };

    class Body {
    public:
        Maths::fvec2 position, velocity, acceleration;
        float mass = 1.0f;
    private:
        BodyType type = BodyType::NONE;
        class World* world = nullptr;

        std::unique_ptr<Shape> shape;

    public:
        Body(const Maths::fvec2& p, float m, BodyType type, World* world, std::unique_ptr<Shape>&& shape)
            : position(p), mass(m), type(type), world(world), shape(std::move(shape)) {}

        void AddVelocity(const Maths::fvec2& vel) { velocity += vel; }
        void Stop() { velocity = 0; }

        [[nodiscard]] OPENGL_API Collision::Event CollidesWith(const Body& target) const;
        [[nodiscard]] OPENGL_API Collision::Event CollidesWith(const Shape& target, const Transform& t) const;
        [[nodiscard]] OPENGL_API Transform GetTransform() const;

        OPENGL_API void Update(float dt);

        [[nodiscard]] const Shape& GetShape() const { return *shape; }
        template <class S> const S& ShapeAs() const { return *dynamic_cast<const S*>(&GetShape()); }

        [[nodiscard]] bool IsStatic()  const { return type == BodyType::STATIC; }
        [[nodiscard]] bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        [[nodiscard]] Maths::rect2f ComputeBoundingBox() const;

        friend class World;
        friend void Collision::StaticResolve (Body&, Body&, const Collision::Event&);
        friend void Collision::DynamicResolve(Body&, Body&, const Collision::Event&);
    };

    struct BodyCreateOptions {
        Maths::fvec2 position {};
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
    };
} // Physics2D
