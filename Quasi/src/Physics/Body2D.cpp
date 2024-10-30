#include "Body2D.h"

#include "Collision2D.h"
#include "World2D.h"

namespace Quasi::Physics2D {
    void Body::AddMomentum(const fVector2& newtonSeconds) {
        velocity += newtonSeconds * invMass;
    }

    // void Body::AddForce(const fVector2& newton) {
    //     acceleration += newton * invMass;
    // }

    void Body::AddAngularMomentum(float angMomentum) {
        angularVelocity += angMomentum * invInertia;
    }

    // void Body::AddTorque(float torque) {
    //     angularAcceleration += torque * invInertia;
    // }

    // void Body::AddRelativeForceToMass(const fVector2& msPosition, const fVector2& force) {
    //     AddForce(force);
    //     AddTorque(msPosition.zcross(force));
    // }

    // void Body::AddRelativeForce(const fVector2& relPosition, const fVector2& force) {
    //     return AddRelativeForceToMass(relPosition - centerOfMass, force);
    // }
    //
    // void Body::AddForceAt(const fVector2& absPosition, const fVector2& force) {
    //     return AddRelativeForce(absPosition - position, force);
    // }

    void Body::SetMass(float newMass) {
        inertia *= newMass / mass;
        invInertia = inertia > 0 ? 1 / inertia : 0;
        mass = newMass;
        invMass = mass > 0 ? 1 / mass : 0;
    }

    Manifold Body::CollideWith(const Body& target) const {
        return CollideWith(target.shape, target.GetTransform());
    }

    Manifold Body::CollideWith(const Shape& target, const PhysicsTransform& xf) const {
        return CollideShapes(shape, GetTransform(), target, xf);
    }

    bool Body::OverlapsWith(const Body& target) const {
        return OverlapsWith(target.shape, target.GetTransform());
    }

    bool Body::OverlapsWith(const Shape& target, const PhysicsTransform& xf) const {
        return OverlapShapes(shape, GetTransform(), target, xf);
    }

    PhysicsTransform Body::GetTransform() const {
        return { position, rotation };
    }

    void Body::Update(float dt) {
        position += velocity * dt;
        rotation *= fComplex::rotate(angularVelocity * dt);
        TryUpdateTransforms();
    }

    void Body::TryUpdateTransforms() {
        if (shapeHasChanged) {
            baseBoundingBox = shape.ComputeBoundingBox();
            centerOfMass = shape.CenterOfMass();
            inertia = shape.Inertia() * mass;
            invInertia = inertia > 0 ? 1 / inertia : 0;
            shapeHasChanged = false;
        }
        boundingBox = GetTransform().TransformRect(baseBoundingBox);
    }

    void Body::SetShapeHasChanged() {
        shapeHasChanged = true;
    }

    fRect2D Body::BoundingBox() const {
        return boundingBox;
    }

    BodyHandle::BodyHandle(Body& b) : index(&b - b.world->bodies.data()), world(b.world) {}

    Body* BodyHandle::Address() { return world ? world->BodyAt(index).Address() : nullptr; }
    const Body* BodyHandle::Address() const { return world ? world->BodyAt(index).Address() : nullptr; }
    Ref<Body> BodyHandle::Reference() { return DerefPtr(Address()); }
    Ref<const Body> BodyHandle::Reference() const { return DerefPtr(Address()); }

    void BodyHandle::Remove() {
        world->DeleteBody(index);
    }
} // Physics2D