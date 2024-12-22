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
        Ref<World> world;
        bool enabled = true;
        bool shapeHasChanged = true;

        Shape shape;
        fRect2D baseBoundingBox;

        Body(const fVector2& p, const fComplex& r, float m, BodyType type, World& world, Shape shape)
            : position(p), rotation(r), mass(m), invMass(m > 0 ? 1 / m : 0), type(type), world(world),
              shape(std::move(shape)) { TryUpdateTransforms(); }

        void AddVelocity       (const fVector2& vel) { velocity += vel; }
        void AddMomentum       (const fVector2& newtonSeconds);
        void AddAngularVelocity(float angVel) { angularVelocity += angVel; }
        void AddAngularMomentum(float angMomentum);

        void AddRelativeVelocity(const fVector2& relPosition, const fVector2& vel);
        void AddVelocityAt      (const fVector2& absPosition, const fVector2& vel);

        void SetMass(float newMass);

        void Stop() { velocity = 0; angularVelocity = 0; }

        Manifold CollideWith(const Body& target) const;
        Manifold CollideWith(const Shape& target, const PhysicsTransform& xf) const;
        bool OverlapsWith(const Body& target) const;
        bool OverlapsWith(const Shape& target, const PhysicsTransform& xf) const;
        PhysicsTransform GetTransform() const;

        void Update(float dt);
        void TryUpdateTransforms();
        void SetShapeHasChanged();

        bool IsStatic()  const { return type == BodyType::STATIC; }
        bool IsDynamic() const { return type == BodyType::DYNAMIC; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }

        fRect2D BoundingBox() const;

        friend class World;
        friend void StaticResolve (Body&, Body&, const Manifold&);
        friend void DynamicResolve(Body&, Body&, const Manifold&);
    };

    struct BodyHandle : INullable<Body&, BodyHandle>, IReference<Body, BodyHandle> {
        u32 index;
        OptRef<World> world;

        using IReference::operator->;
        using IReference::operator*;

    private:
        BodyHandle(u32 i, OptRef<World> w) : index(i), world(w) {}
    public:
        BodyHandle() = default;
        BodyHandle(Nullptr) : BodyHandle() {}
        BodyHandle(Body& b);
        BodyHandle(const Body& b) : BodyHandle(Memory::AsMut(b)) {}

        static BodyHandle At(World& w, u32 i) { return { i, w }; }

        Body& ValueImpl();
        const Body& ValueImpl() const;
        Body& UnwrapImpl() { return Value(); }
        const Body& UnwrapImpl() const { return Value(); }

        bool HasValueImpl() const;

        static BodyHandle NoneImpl() { return {}; }
        static BodyHandle SomeImpl(const Body& b) { return { b }; }

        void Remove();
    };

    struct BodyCreateOptions {
        fVector2 position {};
        float rotAngle = 0.0f;
        BodyType type = BodyType::DYNAMIC;
        float density = 1.0f;
    };
} // Physics2D
