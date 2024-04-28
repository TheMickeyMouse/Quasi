#include "Transform2D.h"

namespace Physics2D {
    void Transform::SetAngle(float angle) {
        rotation = Maths::fvec2::from_polar(1.0f, angle);
    }

    Maths::fvec2 Transform::RotationMul(const Maths::fvec2& a, const Maths::fvec2& b) {
        return { a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x };
    }

    Transform Transform::Inverse() const {
        return { -position, { rotation.x, -rotation.y } };
    }

    Transform Transform::operator*(const Transform& t) const {
        return { position + rotation * t.position, RotationMul(rotation, t.rotation) };
    }

    Maths::fvec2 Transform::operator*(const Maths::fvec2& p) const {
        return p.x * rotation - p.y * rotation.perpend() + position;
    }

    Maths::rect2f Transform::operator*(const Maths::rect2f& r) const {
        return Maths::rect2f { (*this) * r.min, (*this) * r.max }.corrected();
    }
} // Physics2D