#pragma once
#include "Collision2D.h"

#include "Shape2D.h"
#include "PhysicsTransform2D.h"
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
        void AddAcceleration(const Math::fVector2& acc) { acceleration += acc; }
        void AddMomentum(const Math::fVector2& newtonSeconds);
        void AddForce(const Math::fVector2& newton);
        void Stop() { velocity = 0; }

        [[nodiscard]] Manifold CollideWith(const Body& target) const;
        [[nodiscard]] Manifold CollideWith(const Shape& target, const PhysicsTransform& t) const;
        [[nodiscard]] bool OverlapsWith(const Body& target) const;
        [[nodiscard]] bool OverlapsWith(const Shape& target, const PhysicsTransform& t) const;
        [[nodiscard]] PhysicsTransform GetTransform() const;

        void Update(float dt);

        [[nodiscard]] bool IsStatic()  const { return type == BodyType::STATIC; }
        [[nodiscard]] bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }

        [[nodiscard]] Math::fVector2 CenterOfMass() const;
        void ShiftOriginToMassCenter();
        [[nodiscard]] Math::fRect2D ComputeBoundingBox() const;

        friend class World;
        friend void Collision::StaticResolve (Body&, Body&, const Manifold&);
        friend void Collision::DynamicResolve(Body&, Body&, const Manifold&);
    };

    struct BodyCreateOptions {
        Math::fVector2 position {};
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
    };
} // Physics2D
