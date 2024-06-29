#include "Transform2D.h"

namespace Quasi::Physics2D {
    void Transform::SetAngle(float angle) {
        rotation = Math::fVector2::from_polar(1.0f, angle);
    }

    Math::fVector2 Transform::RotationMul(const Math::fVector2& a, const Math::fVector2& b) {
        return { a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x };
    }

    Transform Transform::Inverse() const {
        return { -position, { rotation.x, -rotation.y } };
    }

    Transform Transform::operator*(const Transform& t) const {
        return { position + rotation * t.position, RotationMul(rotation, t.rotation) };
    }

    Math::fVector2 Transform::operator*(const Math::fVector2& p) const {
        return p.x * rotation - p.y * rotation.perpend() + position;
    }

    Math::fRect2D Transform::operator*(const Math::fRect2D& r) const {
        return Math::fRect2D { (*this) * r.min, (*this) * r.max }.corrected();
    }
} // Physics2D