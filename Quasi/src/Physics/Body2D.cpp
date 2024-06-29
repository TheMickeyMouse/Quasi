#include "Body2D.h"

#include "Collision2D.h"

namespace Quasi::Physics2D {
    Collision::Event Body::CollidesWith(const Body& target) const {
        return CollidesWith(*target.shape, target.GetTransform());
    }

    Collision::Event Body::CollidesWith(const Shape& target, const Transform& t) const {
        return Collision::Between(*shape, GetTransform(), target, t);
    }

    Transform Body::GetTransform() const {
        return { position };
    }

    void Body::Update(float dt) {
        velocity += acceleration * dt;
        position += velocity * dt;
    }

    Math::fRect2D Body::ComputeBoundingBox() const {
        return GetTransform() * shape->ComputeBoundingBox();
    }
} // Physics2D