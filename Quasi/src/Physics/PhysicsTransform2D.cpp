#include "PhysicsTransform2D.h"
#include "Math/Transform2D.h"

namespace Quasi::Physics2D {
    PhysicsTransform PhysicsTransform::Translate(const fv2& p) { return { p }; }
    PhysicsTransform PhysicsTransform::Rotation(const Rotation2D& q) { return { 0, q }; }

    fv2 PhysicsTransform::Transform(const fv2& point) const {
        return point.RotateBy(rotation) + position;
    }

    fv2 PhysicsTransform::TransformDir(const fv2& dir) const {
        return dir.RotateBy(rotation);
    }

    fv2 PhysicsTransform::TransformInverse(const fv2& point) const {
        return rotation.InvRotate(point - position);
    }

    fv2 PhysicsTransform::TransformInverseDir(const fv2& dir) const {
        return rotation.InvRotate(dir);
    }

    fLine2D PhysicsTransform::TransformLine(const fLine2D& line) const {
        return { Transform(line.start), TransformDir(line.forward) };
    }

    fRect2D PhysicsTransform::TransformRect(const fRect2D& rect) const {
        fv2 c = TransformDir(rect.Center());
        const fv2 p1 = TransformDir(rect.Corner({ false, false })), p2 = TransformDir(rect.Corner({ true, false }));
        const fv2 diff = { std::max(std::abs(p1.x - c.x), std::abs(p2.x - c.x)),
                           std::max(std::abs(p1.y - c.y), std::abs(p2.y - c.y)) };
        c += position;
        return { c - diff, c + diff };
    }

    Transform2D PhysicsTransform::AsObjectTransform(const fv2& scale) const {
        return { position, scale, rotation };
    }

    PhysicsTransform PhysicsTransform::Inverse() const {
        return { -rotation.InvRotate(position), rotation.Inverse() };
    }

    PhysicsTransform PhysicsTransform::Applied(const PhysicsTransform& transformer) const {
        return { position.RotateBy(transformer.rotation) + transformer.position, rotation + transformer.rotation };
    }
    PhysicsTransform& PhysicsTransform::Apply(const PhysicsTransform& transformer) {
        position = position.RotateBy(transformer.rotation) += transformer.position;
        rotation += transformer.rotation;
        return *this;
    }

    void PhysicsTransform::Reset() {
        position = 0;
        rotation = {};
    }
} // Physics2D