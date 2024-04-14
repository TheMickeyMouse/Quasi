#include "CircleCollider2D.h"

namespace Physics2D {
    bool CircleCollider::CollidesWith(const Collider& other) const {
        if (const auto* c = dynamic_cast<const CircleCollider*>(&other)) {
            return CollideCircle(*c);
        }
        return false;
    }

    bool CircleCollider::CollideCircle(const CircleCollider& other) const {
        return position.distsq(other.position) < (radius + other.radius) * (radius + other.radius);
    }

    bool CircleCollider::CollidePoint(const Maths::fvec2& other) const {
        return position.distsq(other) < radius * radius;
    }

    void CircleCollider::StaticResolve(CircleCollider& other) {
        const float overlap = radius + other.radius - position.dist(other.position);
        const Maths::fvec2 direction = (position - other.position).norm();
        position += direction * (overlap * 0.5f);
        other.position -= direction * (overlap * 0.5f);
    }

    void CircleCollider::DynamicResolve(CircleCollider& other) {
        const Maths::fvec2 normal = (other.position - position).norm(),
                           tangent = -normal.perpend();
        const float dnorm1 = velocity.dot(normal), dnorm2 = other.velocity.dot(normal);
        const float momentum1 = (dnorm1 * (mass - other.mass) + 2.0f * other.mass * dnorm2) / (mass + other.mass);
        const float momentum2 = (dnorm2 * (other.mass - mass) + 2.0f * mass       * dnorm1) / (mass + other.mass);
        velocity       =       velocity.dot(tangent) * tangent + momentum1 * normal;
        other.velocity = other.velocity.dot(tangent) * tangent + momentum2 * normal;
    }
} // Physics2D