#pragma once
#include "Collision2D.h"

#include "IShape2D.h"
#include "PhysicsTransform2D.h"
#include "Shape2D.h"
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
        float restitution = 0.8f;
        u32 sortedIndex = 0;
        BodyType type = BodyType::NONE;
        bool enabled = true;
        Ref<World> world = nullptr;

        Shape shape;
        TransformedShape transformedShape;
        Math::fRect2D boundingBox;

        Body(const Math::fVector2& p, float m, float restitution, BodyType type, Ref<World> world, Shape shape)
            : position(p), mass(m), restitution(restitution), type(type), world(world),
              shape(std::move(shape)) { UpdateTransformShape(); }

        void AddVelocity(const Math::fVector2& vel) { velocity += vel; }
        void AddAcceleration(const Math::fVector2& acc) { acceleration += acc; }
        void AddMomentum(const Math::fVector2& newtonSeconds);
        void AddForce(const Math::fVector2& newton);
        void Stop() { velocity = 0; }

        [[nodiscard]] Manifold CollideWith(const Body& target) const;
        [[nodiscard]] Manifold CollideWith(const TransformedShape& target) const;
        [[nodiscard]] bool OverlapsWith(const Body& target) const;
        [[nodiscard]] bool OverlapsWith(const TransformedShape& target) const;
        [[nodiscard]] PhysicsTransform GetTransform() const;

        void Update(float dt);
        void UpdateTransformShape();

        [[nodiscard]] bool IsStatic()  const { return type == BodyType::STATIC; }
        [[nodiscard]] bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }

        [[nodiscard]] Math::fVector2 CenterOfMass() const;
        void ShiftOriginToMassCenter();
        [[nodiscard]] Math::fRect2D BoundingBox() const;

        friend class World;
        friend void StaticResolve (Body&, Body&, const Manifold&);
        friend void DynamicResolve(Body&, Body&, const Manifold&);
    };

    struct BodyHandle {
        u32 index;
        Ref<World> world;

        BodyHandle(std::nullptr_t) : index(0), world(nullptr) {}
        BodyHandle() : BodyHandle(nullptr) {}
        BodyHandle(Body& b);
        BodyHandle(const Body& b) : BodyHandle(const_cast<Body&>(b)) {}
        BodyHandle(u32 i, World& w) : index(i), world(w) {}

        static BodyHandle At(World& w, u32 i) { return { i, w }; }

        Body* Address();
        [[nodiscard]] const Body* Address() const;
        Ref<Body> Reference();
        [[nodiscard]] Ref<const Body> Reference() const;

        [[nodiscard]] operator bool() const { return world; }
        operator Body&() { return *Address(); }
        [[nodiscard]] operator const Body&() const { return *Address(); }
        Body* operator->() { return Address(); }
        [[nodiscard]] const Body* operator->() const { return Address(); }

        void Remove();
    };

    struct BodyCreateOptions {
        Math::fVector2 position {};
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
        float restitution = 0.8f;
    };
} // Physics2D
