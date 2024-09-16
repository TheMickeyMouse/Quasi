#include "Body2D.h"

#include "Collision2D.h"
#include "World2D.h"

namespace Quasi::Physics2D {
    void Body::AddMomentum(const Math::fVector2& newtonSeconds) {
        velocity += newtonSeconds / mass;
    }

    void Body::AddForce(const Math::fVector2& newton) {
        acceleration += newton / mass;
    }

    Manifold Body::CollideWith(const Body& target) const {
        return CollideWith(target.shape, target.GetTransform());
    }

    Manifold Body::CollideWith(const Shape& target, const PhysicsTransform& t) const {
        return CollideShapes(shape, GetTransform(), target, t);
    }

    bool Body::OverlapsWith(const Body& target) const {
        return OverlapsWith(target.shape, target.GetTransform());
    }

    bool Body::OverlapsWith(const Shape& target, const PhysicsTransform& t) const {
        return OverlapShapes(shape, GetTransform(), target, t);
    }

    PhysicsTransform Body::GetTransform() const {
        return { position };
    }

    void Body::Update(float dt) {
        velocity += acceleration * dt;
        position += velocity * dt;
    }

    Math::fVector2 Body::CenterOfMass() const {
        return shape.CenterOfMass() + position;
    }

    void Body::ShiftOriginToMassCenter() {
        position += shape.CenterOfMass();
    }

    Math::fRect2D Body::ComputeBoundingBox() const {
        return GetTransform() * shape.ComputeBoundingBox();
    }

    BodyHandle::BodyHandle(Body& b) : index(&b - b.world->bodies.data()), world(b.world) {}

    Body& BodyHandle::Value() { return world->bodies[index]; }
    const Body& BodyHandle::Value() const { return world->bodies[index]; }
    Body* BodyHandle::Address() { return &world->bodies[index]; }
    const Body* BodyHandle::Address() const { return &world->bodies[index]; }
} // Physics2D