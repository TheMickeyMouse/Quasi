#include "Body2D.h"

#include "Collision2D.h"

namespace Quasi::Physics2D {
    void Body::AddMomentum(const Math::fVector2& newtonSeconds) {
        velocity += newtonSeconds / mass;
    }

    void Body::AddForce(const Math::fVector2& newton) {
        acceleration += newton / mass;
    }

    Manifold Body::CollideWith(const Body& target) const {
        return CollideWith(*target.shape, target.GetTransform());
    }

    Manifold Body::CollideWith(const Shape& target, const PhysicsTransform& t) const {
        return Collision::CollideShapes(*shape, GetTransform(), target, t);
    }

    bool Body::OverlapsWith(const Body& target) const {
        return OverlapsWith(target.shape, target.GetTransform());
    }

    bool Body::OverlapsWith(const Shape& target, const PhysicsTransform& t) const {
        return Collision::OverlapShapes(shape, GetTransform(), target, t);
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