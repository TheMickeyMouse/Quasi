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
        fVector2 position, velocity;
        fComplex rotation; float angularVelocity = 0.0f;
        float mass = 1.0f, invMass = 1.0f, inertia = 1.0f, invInertia = 1.0f;
        fRect2D boundingBox;
        u32 sortedIndex = 0;
        BodyType type = BodyType::NONE;
        Ref<World> world = nullptr;
        bool enabled = true;
        bool shapeHasChanged = true;

        Shape shape;
        fRect2D baseBoundingBox;
        fVector2 centerOfMass;

        Body(const fVector2& p, const fComplex& r, float m, BodyType type, Ref<World> world, Shape shape)
            : position(p), rotation(r), mass(m), invMass(m > 0 ? 1 / m : 0), type(type), world(world),
              shape(std::move(shape)) { TryUpdateTransforms(); }

        void AddVelocity    (const fVector2& vel) { velocity += vel; }
        void AddMomentum    (const fVector2& newtonSeconds);
        void AddAngularVelocity    (float angVel) { angularVelocity += angVel; }
        void AddAngularMomentum    (float angMomentum);

        // void AddRelativeForceToMass(const fVector2& msPosition,  const fVector2& force);
        // void AddRelativeForce      (const fVector2& relPosition, const fVector2& force);
        // void AddForceAt            (const fVector2& absPosition, const fVector2& force);

        void SetMass(float newMass);

        void Stop() { velocity = 0; angularVelocity = 0; }

        [[nodiscard]] Manifold CollideWith(const Body& target) const;
        [[nodiscard]] Manifold CollideWith(const Shape& target, const PhysicsTransform& xf) const;
        [[nodiscard]] bool OverlapsWith(const Body& target) const;
        [[nodiscard]] bool OverlapsWith(const Shape& target, const PhysicsTransform& xf) const;
        [[nodiscard]] PhysicsTransform GetTransform() const;

        void Update(float dt);
        void TryUpdateTransforms();
        void SetShapeHasChanged();

        [[nodiscard]] bool IsStatic()  const { return type == BodyType::STATIC; }
        [[nodiscard]] bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }

        [[nodiscard]] fRect2D BoundingBox() const;

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
        fVector2 position {};
        float rotAngle = 0.0f;
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
    };
} // Physics2D
