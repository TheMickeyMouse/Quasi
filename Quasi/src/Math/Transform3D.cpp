#include "Transform3D.h"

namespace Quasi::Math {
    void Transform3D::Translate(const fVector3& p) { position += p; }
    void Transform3D::Scale(const fVector3& s) { scale *= s; }
    void Transform3D::Rotate(const fVector3& r) { Rotate(Quaternion::from_euler(r)); }
    void Transform3D::Rotate(const Quaternion& q) { rotation.rotate_by(q); }
    void Transform3D::RotateX(float r) { Rotate(Quaternion::rotate_x(r)); }
    void Transform3D::RotateY(float r) { Rotate(Quaternion::rotate_y(r)); }
    void Transform3D::RotateZ(float r) { Rotate(Quaternion::rotate_z(r)); }

    Transform3D Transform3D::Translated(const fVector3& p) const {
        return { position + p, scale, rotation };
    }
    Transform3D Transform3D::Scaled(const fVector3& s) const {
        return { position, scale * s, rotation };
    }
    Transform3D Transform3D::Rotated(const fVector3& r) const {
        return Rotated(Quaternion::from_euler(r));
    }
    Transform3D Transform3D::Rotated(const Quaternion& q) const {
        return { position, scale, rotation.then(q) };
    }
    Transform3D Transform3D::RotatedX(float r) const { return Rotated(Quaternion::rotate_x(r)); }
    Transform3D Transform3D::RotatedY(float r) const { return Rotated(Quaternion::rotate_y(r)); }
    Transform3D Transform3D::RotatedZ(float r) const { return Rotated(Quaternion::rotate_z(r)); }

    Transform3D Transform3D::Translation(const fVector3& p) { return { p }; }
    Transform3D Transform3D::Scaling(const fVector3& s)     { return { 0, s }; }
    Transform3D Transform3D::Rotation(const fVector3& r)    { return Rotation(Quaternion::from_euler(r)); }
    Transform3D Transform3D::Rotation(const Quaternion& q)  { return { 0, 1, q }; }

    Transform3D Transform3D::NormalTransform() const {
        return { 0, 1.0f / scale, rotation };
    }

    fVector3 Transform3D::Transform(const fVector3& point) const {
        return (point * scale).rotated_by(rotation) + position;
    }

    void Transform3D::TransformInplace(fVector3& point) const {
        point *= scale;
        point.rotate_by(rotation);
        point += position;
    }

    fVector3 Transform3D::TransformInverse(const fVector3& point) const {
        return rotation.invrotate(point - position) / scale;
    }

    void Transform3D::TransformInverseInplace(fVector3& point) const {
        point -= position;
        point = rotation.invrotate(point);
        point /= scale;
    }

    fVector3 Transform3D::TransformNormal(const fVector3& normal) const {
        return (normal / scale).rotated_by(rotation).norm();
    }

    void Transform3D::TransformNormalInplace(fVector3& normal) const {
        normal /= scale;
        normal.rotate_by(rotation);
        normal /= normal.len();
    }

    fVector3 Transform3D::TransformInverseNormal(const fVector3& normal) const {
        return (rotation.invrotate(normal) * scale).norm();
    }

    void Transform3D::TransformInverseNormalInplace(fVector3& normal) const {
        normal = rotation.invrotate(normal);
        normal *= scale;
        normal /= normal.len();
    }

    Transform3D Transform3D::Applied(const Transform3D& transformer) const {
        return { transformer * position, transformer.scale * scale, rotation.then(transformer.rotation) };
    }

    Transform3D& Transform3D::Apply(const Transform3D& transformer) {
        transformer.TransformInplace(position);
        scale *= transformer.scale;
        rotation = rotation.then(transformer.rotation);
        return *this;
    }

    void Transform3D::Reset() {
        position = 0;
        scale = 1;
        rotation = 1;
    }

    Matrix3x3 Transform3D::LinearMatrix() const {
        Matrix3x3 rotmatrix = rotation.as_matrix();
        rotmatrix.mat.x *= scale.x;
        rotmatrix.mat.y *= scale.y;
        rotmatrix.mat.z *= scale.z;
        return rotmatrix;
    }

    Matrix3D Transform3D::TransformMatrix() const {
        Matrix3D rotmatrix = rotation.as_rotation_matrix();
        rotmatrix.mat.x *= scale.x;
        rotmatrix.mat.y *= scale.y;
        rotmatrix.mat.z *= scale.z;
        rotmatrix.mat.w = position.with_w(1);
        return rotmatrix;
    }
} // Quasi