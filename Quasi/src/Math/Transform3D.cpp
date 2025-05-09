#include "Transform3D.h"
#include "Random.h"

namespace Quasi::Math {
    fv3 Rotation3D::IHat() const {
        return { 1 - 2 * (y * y + z * z),
                     2 * (x * y + z * w),
                     2 * (x * z - y * w), };
    }
    fv3 Rotation3D::JHat() const {
        return {     2 * (x * y - z * w),
                 1 - 2 * (x * x + z * z),
                     2 * (y * z + x * w), };
    }
    fv3 Rotation3D::KHat() const {
        return {     2 * (x * z + y * w),
                     2 * (y * z - x * w),
                 1 - 2 * (x * x + y * y), };
    }

    Radians Rotation3D::AngleBetween(const Rotation3D& r) const { return Arccos(Dot(r)); }

    Rotation3D Rotation3D::RotateBy(const Rotation3D& r) const {
        // 14 muls, 10 adds
        return Quaternion {
            2 * w * r.w - 1,
            w * r.x + r.w * x + y * r.z - z * r.y,
            w * r.y + r.w * y + z * r.x - x * r.z,
            w * r.z + r.w * z + x * r.y - y * r.x
        };
    }
    Rotation3D Rotation3D::RotateByInv(const Rotation3D& r) const {
        // r.xyz signs are flipped
        return Quaternion {
            2 * w * r.w - 1,
            r.w * x - w * r.x - y * r.z + z * r.y,
            r.w * y - w * r.y - z * r.x + x * r.z,
            r.w * z - w * r.z - x * r.y + y * r.x
        };
    }

    Rotation3D Rotation3D::Halved() const {
        const float u = std::sqrt((1 - w) * 0.5f);
        return Quaternion { std::sqrt((1 + w) * 0.5f), x * u, y * u, z * u };
    }
    Rotation3D Rotation3D::Doubled() const {
        const float w2 = 2 * w;
        return Quaternion { w2 * w - 1, w2 * x, w2 * y, w2 * z };
    }
    Rotation3D Rotation3D::Tripled() const {
        return Doubled() + (*this);
    }
    fv3 Rotation3D::Rotate(const fv3& v) const {
        // 21 mul, 18 adds
        const float xx = x * x, yy = y * y, zz = z * z,
                    xy = x * y, xz = x * z, yz = y * z, xw = x * w, yw = y * w, zw = z * w;
        // [ 1 - 2 * (yy + zz),     2 * (xy - zw),     2 * (xz + yw) ]
        // [     2 * (xy + zw), 1 - 2 * (xx + zz),     2 * (yz - xw) ]
        // [     2 * (xz - yw),     2 * (yz + xw), 1 - 2 * (xx + yy) ]
        return {
            2 * (v.y * (xy - zw) + v.z * (xz + yw) - v.x * (yy + zz)) + v.x,
            2 * (v.x * (xy + zw) + v.z * (yz - xw) - v.y * (xx + zz)) + v.y,
            2 * (v.x * (xz - yw) + v.y * (yz + xw) - v.z * (xx + yy)) + v.z,
        };
    }

    fv3 Rotation3D::InvRotate(const fv3& v) const {
        // 21 mul, 18 adds
        // xyz signs are flipped, but since theyre multiplied by pairs the only
        // sign change is in xw, yw, and zw
        const float xx = x * x, yy = y * y, zz = z * z,
                    xy = x * y, xz = x * z, yz = y * z, xw = x * w, yw = y * w, zw = z * w;
        return {
            2 * (v.y * (xy + zw) + v.z * (xz - yw) - v.x * (yy + zz)) + v.x,
            2 * (v.x * (xy - zw) + v.z * (yz + xw) - v.y * (xx + zz)) + v.y,
            2 * (v.x * (xz + yw) + v.y * (yz - xw) - v.z * (xx + yy)) + v.z,
        };
    }

    Rotation3D Rotation3D::Random(RandomGenerator& rg) {
        const auto [u, v, w] = fv3::Random(rg, 0, 1);
        const f32 uComp = std::sqrt(1 - u), uSqrt = std::sqrt(u);
        return Quaternion {
            uComp * std::sin(TAU * v),
            uComp * std::cos(TAU * v),
            uSqrt * std::sin(TAU * w),
            uSqrt * std::cos(TAU * w)
        };
    }

    Transform3D Transform3D::Translate(const fv3& p)        { return { p }; }
    Transform3D Transform3D::Scale    (const fv3& s)        { return { 0, s }; }
    Transform3D Transform3D::Rotation (const Rotation3D& r) { return { 0, 1, r }; }

    Transform3D Transform3D::NormalTransform() const {
        return { 0, 1.0f / scale, rotation };
    }

    fv3 Transform3D::Transform(const fv3& point) const {
        return rotation.Rotate(point * scale) + position;
    }
    fv3 Transform3D::TransformInverse(const fv3& point) const {
        return rotation.InvRotate(point - position) / scale;
    }
    fv3 Transform3D::TransformNormal(const fv3& normal) const {
        return rotation.Rotate(normal / scale).Norm();
    }
    fv3 Transform3D::TransformInverseNormal(const fv3& normal) const {
        return rotation.InvRotate(normal * scale).Norm();
    }

    Transform3D Transform3D::Applied(const Transform3D& transformer) const {
        return { transformer * position, transformer.scale * scale, transformer.rotation + rotation };
    }

    Transform3D& Transform3D::Apply(const Transform3D& transformer) {
        position = transformer.Transform(position);
        scale    *= transformer.scale;
        rotation = transformer.rotation + rotation; // order matters
        return *this;
    }

    void Transform3D::Reset() {
        position = 0;
        scale = 1;
        rotation = {};
    }

    Matrix3x3 Transform3D::LinearMatrix() const {
        Matrix3x3 rotmatrix = rotation.AsMatrixLinear();
        rotmatrix.unitVectors[0] *= scale.x;
        rotmatrix.unitVectors[1] *= scale.y;
        rotmatrix.unitVectors[2] *= scale.z;
        return rotmatrix;
    }

    Matrix3D Transform3D::TransformMatrix() const {
        Matrix3D rotmatrix = rotation.AsMatrix();
        rotmatrix.unitVectors[0] *= scale.x;
        rotmatrix.unitVectors[1] *= scale.y;
        rotmatrix.unitVectors[2] *= scale.z;
        rotmatrix.unitVectors[3] = position.AddW(1);
        return rotmatrix;
    }
} // Quasi