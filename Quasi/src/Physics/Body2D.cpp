#include "Body2D.h"

#include "Collision2D.h"

namespace Quasi::Physics2D {
    void Body::AddMomentum(const Math::fVector2& newtonSeconds) {
        velocity += newtonSeconds / mass;
    }

    void Body::AddForce(const Math::fVector2& newton) {
        acceleration += newton / mass;
    }

    Collision::Event Body::CollidesWith(const Body& target) const {
        return CollidesWith(*target.shape, target.GetTransform());
    }

    Collision::Event Body::CollidesWith(const Shape& target, const PhysicsTransform& t) const {
        return Collision::CollideShapeDyn(*shape, GetTransform(), target, t);
    }

    PhysicsTransform Body::GetTransform() const {
        return { position };
    }

    void Body::Update(float dt) {
        velocity += acceleration * dt;
        position += velocity * dt;
    }

    Math::fVector2 Body::CenterOfMass() const {
        return shape->CenterOfMass() + position;
    }

    void Body::ShiftOriginToMassCenter() {
        position += shape->CenterOfMass();
    }

    Math::fRect2D Body::ComputeBoundingBox() const {
        return GetTransform() * shape->ComputeBoundingBox();
    }
} // Physics2D