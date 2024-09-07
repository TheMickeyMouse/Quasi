#include "PhysicsTransform2D.h"
#include "Math/Transform2D.h"

namespace Quasi::Physics2D {
    void PhysicsTransform::Translate(const Math::fVector2& p) { position += p; }
    void PhysicsTransform::Rotate(float r)             { Rotate(Math::fComplex::rotate(r)); }
    void PhysicsTransform::Rotate(const Math::fComplex& r)    { rotation *= r; }

    PhysicsTransform PhysicsTransform::Translated(const Math::fVector2& p) const {
        return { position + p, rotation };
    }
    PhysicsTransform PhysicsTransform::Rotated(float r) const {
        return Rotated(Math::fComplex::rotate(r));
    }
    PhysicsTransform PhysicsTransform::Rotated(const Math::fComplex& r) const {
        return { position, rotation * r };
    }

    PhysicsTransform PhysicsTransform::Translation(const Math::fVector2& p) { return { p }; }
    PhysicsTransform PhysicsTransform::Rotation(float r) { return Rotation(Math::fComplex::rotate(r)); }
    PhysicsTransform PhysicsTransform::Rotation(const Math::fComplex& q) { return { 0, q }; }

    Math::fVector2 PhysicsTransform::Transform(const Math::fVector2& point) const {
        return point.rotated_by(rotation) + position;
    }

    Math::fVector2 PhysicsTransform::TransformOffset(const Math::fVector2& offset) const {
        return offset.rotated_by(rotation);
    }

    void PhysicsTransform::TransformInplace(Math::fVector2& point) const {
        point.rotate_by(rotation) += position;
    }
    Math::fVector2 PhysicsTransform::TransformInverse(const Math::fVector2& point) const {
        return (point - position).rotated_by(rotation.inv());
    }
    void PhysicsTransform::TransformInverseInplace(Math::fVector2& point) const {
        (point -= position).rotate_by(rotation.inv());
    }

    Math::fRect2D PhysicsTransform::TransformRect(const Math::fRect2D& rect) const {
        return Math::fRect2D { Transform(rect.min), Transform(rect.max) }.corrected();
    }

    Math::Transform2D PhysicsTransform::AsObjectTransform(const Math::fVector2& scale) const {
        return { position, scale, rotation };
    }

    PhysicsTransform PhysicsTransform::Inverse() const {
        return { -position.rotated_by(rotation), rotation.inv() };
    }

    PhysicsTransform PhysicsTransform::Applied(const PhysicsTransform& transformer) const {
        return { position.rotated_by(transformer.rotation) + transformer.position, rotation * transformer.rotation };
    }
    PhysicsTransform& PhysicsTransform::Apply(const PhysicsTransform& transformer) {
        position.rotate_by(transformer.rotation) += transformer.position;
        rotation *= transformer.rotation;
        return *this;
    }

    void PhysicsTransform::Reset() {
        position = 0;
        rotation = 1;
    }
} // Physics2D