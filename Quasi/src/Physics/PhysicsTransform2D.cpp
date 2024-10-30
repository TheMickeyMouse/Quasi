#include "PhysicsTransform2D.h"
#include "Math/Transform2D.h"

namespace Quasi::Physics2D {
    void PhysicsTransform::Translate(const fVector2& p) { position += p; }
    void PhysicsTransform::Rotate(float r)              { Rotate(fComplex::rotate(r)); }
    void PhysicsTransform::Rotate(const fComplex& r)    { rotation *= r; }

    PhysicsTransform PhysicsTransform::Translated(const fVector2& p) const {
        return { position + p, rotation };
    }
    PhysicsTransform PhysicsTransform::Rotated(float r) const {
        return Rotated(fComplex::rotate(r));
    }
    PhysicsTransform PhysicsTransform::Rotated(const fComplex& r) const {
        return { position, rotation * r };
    }

    PhysicsTransform PhysicsTransform::Translation(const fVector2& p) { return { p }; }
    PhysicsTransform PhysicsTransform::Rotation(float r) { return Rotation(fComplex::rotate(r)); }
    PhysicsTransform PhysicsTransform::Rotation(const fComplex& q) { return { 0, q }; }

    fVector2 PhysicsTransform::Transform(const fVector2& point) const {
        return point.rotated_by(rotation) + position;
    }

    fVector2 PhysicsTransform::TransformOffset(const fVector2& offset) const {
        return offset.rotated_by(rotation);
    }

    void PhysicsTransform::TransformInplace(fVector2& point) const {
        point.rotate_by(rotation) += position;
    }
    fVector2 PhysicsTransform::TransformInverse(const fVector2& point) const {
        return rotation.invrotate(point - position);
    }

    fVector2 PhysicsTransform::TransformInverseOffset(const fVector2& offset) const {
        return rotation.invrotate(offset);
    }

    void PhysicsTransform::TransformInverseInplace(fVector2& point) const {
        (point -= position).rotate_by(rotation.conj());
    }

    fLine2D PhysicsTransform::TransformLine(const fLine2D& line) const {
        return { Transform(line.start), Transform(line.end) };
    }

    fRect2D PhysicsTransform::TransformRect(const fRect2D& rect) const {
        fVector2 c = TransformOffset(rect.center());
        const fVector2 p1 = TransformOffset(rect.corner(0)), p2 = TransformOffset(rect.corner(1));
        const fVector2 diff = { std::max(std::abs(p1.x - c.x), std::abs(p2.x - c.x)),
                                std::max(std::abs(p1.y - c.y), std::abs(p2.y - c.y)) };
        c += position;
        return { c - diff, c + diff };
    }

    Transform2D PhysicsTransform::AsObjectTransform(const fVector2& scale) const {
        return { position, scale, rotation };
    }

    PhysicsTransform PhysicsTransform::Inverse() const {
        return { -position.rotated_by(rotation), rotation.conj() };
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