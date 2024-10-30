#include "Quaternion.h"

#include "Complex.h"
#include "Constants.h"

namespace Quasi::Math {
    const Quaternion Quaternion::i = { 1, 0, 0 };
    const Quaternion Quaternion::j = { 0, 1, 0 };
    const Quaternion Quaternion::k = { 0, 0, 1 };

    Quaternion Quaternion::rotate_axis(const fVector3& axis, float rotation) {
        return { std::cos(rotation * 0.5f), axis.norm(std::sin(rotation * 0.5f)) };
    }

    Quaternion Quaternion::rotate_axis(const fVector3& axis, fComplex rotation) {
        rotation = rotation.sqrt();
        return { rotation.re, axis.norm(rotation.im) };
    }

    Quaternion Quaternion::rotate_x(float xrot) { return { std::cos(xrot * 0.5f), std::sin(xrot * 0.5f), 0, 0 }; }
    Quaternion Quaternion::rotate_y(float yrot) { return { std::cos(yrot * 0.5f), 0, std::sin(yrot * 0.5f), 0 }; }
    Quaternion Quaternion::rotate_z(float zrot) { return { std::cos(zrot * 0.5f), 0, 0, std::sin(zrot * 0.5f) }; }
    Quaternion Quaternion::rotate_xyz(const fVector3& rotation) {
        return rotate_z(rotation.z).then(rotate_x(rotation.x)).then(rotate_y(rotation.y));
        //  return rotate_z(rotation.z) * (rotate_x(rotation.x)) * (rotate_y(rotation.y));
    }

    Quaternion Quaternion::rotate_to(const fVector3& from, const fVector3& to) {
        return { from.dot(to), from.cross(to) };
    }

    fVector3 Quaternion::xyzrot() const {
        const float a = w - x, b = z + y, c = x + w, d = y - z;
        const float xrot = std::asin(2 * (a * a + b * b) / (a * a + b * b + c * c + d * d) - 1);
        const float posAngle = std::atan2(b, a), negAngle = std::atan2(d, c);
        float yrot = posAngle + negAngle;
        float zrot = posAngle - negAngle;
        yrot += (yrot < -PI ? TAU : 0) + (yrot > PI ? -TAU : 0);
        zrot += (zrot < -PI ? TAU : 0) + (zrot > PI ? -TAU : 0);
        return { -xrot, yrot, zrot };
    }

    Quaternion Quaternion::look_at(const fVector3& fwd, const fVector3& worldFront) {
        return rotate_axis(worldFront.cross(fwd).norm(), std::acos(worldFront.dot(fwd)));
    }

    Matrix3x3 Quaternion::as_matrix() const {
        return { fVector3 { 1 - 2 * (y * y + z * z),     2 * (x * y + z * w),     2 * (x * z - y * w) },
                 fVector3 {     2 * (x * y - z * w), 1 - 2 * (x * x + z * z),     2 * (y * z + x * w) },
                 fVector3 {     2 * (x * z + y * w),     2 * (y * z - x * w), 1 - 2 * (x * x + y * y) } };
    }

    Matrix3D Quaternion::as_rotation_matrix() const {
        // same as following, but its faster this way:
        const Quaternion inverse = conj();
        return { (muli() * inverse).xyz(), (mulj() * inverse).xyz(), (mulk() * inverse).xyz() };

        // also assumes quaternion is unit
        // https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
        // return { fVector4 { 1 - 2 * (y * y + z * z),     2 * (x * y + z * w),     2 * (x * z - y * w), 0 },
        //          fVector4 {     2 * (x * y - z * w), 1 - 2 * (x * x + z * z),     2 * (y * z + x * w), 0 },
        //          fVector4 {     2 * (x * z + y * w),     2 * (y * z - x * w), 1 - 2 * (x * x + y * y), 0 },
        //          fVector4 {                       0,                       0,                       0, 1 } };
    }

    Matrix4x4 Quaternion::as_compute_matrix() const {
        return { wxyz(), muli().wxyz(), mulj().wxyz(), mulk().wxyz() };
    }

    float Quaternion::lensq() const { return w * w + x * x + y * y + z * z; }
    float Quaternion::len() const { return std::sqrt(lensq()); }
    float Quaternion::abs() const { return len(); }
    float Quaternion::distsq(const Quaternion& q) const { return (*this - q).lensq(); }
    float Quaternion::dist(const Quaternion& q) const { return (*this - q).len(); }
    Quaternion Quaternion::norm() const { return *this / len(); }

    Quaternion Quaternion::exp() const {
        const fVector3 ijk = xyz();
        const float r = ijk.len(), e = std::exp(w);
        return { e * std::cos(r), std::sin(r) * ijk.safe_norm() };
    }

    Quaternion Quaternion::log() const {
        const float r = len();
        const auto v = xyz();
        return { std::log(r), std::acos(w / r) * v.safe_norm() };
    }

    Quaternion Quaternion::pow(float p) const {
        return (log() * p).exp();
    }

    Quaternion Quaternion::operator+(const Quaternion& q) const { return { w + q.w, x + q.x, y + q.y, z + q.z }; }
    Quaternion Quaternion::operator-(const Quaternion& q) const { return { w - q.w, x - q.x, y - q.y, z - q.z }; }
    Quaternion& Quaternion::operator+=(const Quaternion& q) { w += q.w; x += q.x; y += q.y; z += q.z; return *this; }
    Quaternion& Quaternion::operator-=(const Quaternion& q) { w -= q.w; x -= q.x; y -= q.y; z -= q.z; return *this; }
    Quaternion& Quaternion::operator*=(float v) { w *= v; x *= v; y *= v; z *= v; return *this; }
    Quaternion& Quaternion::operator/=(float v) { w /= v; x /= v; y /= v; z /= v; return *this; }

    Quaternion Quaternion::conj() const { return { w, -x, -y, -z }; }
    Quaternion Quaternion::inv() const { return conj() / lensq(); }

    Quaternion Quaternion::muli() const { return { -x, w, z, -y }; }
    Quaternion Quaternion::mulj() const { return { -y, -z, w, x }; }
    Quaternion Quaternion::mulk() const { return { -z, y, -x, w }; }

    Quaternion Quaternion::lerp(const Quaternion& q, float t) const {
        return { w + (q.w - w) * t, x + (q.x - x) * t, y + (q.y - y) * t, z + (q.z - z) * t };
    }

    Quaternion Quaternion::slerp(const Quaternion& q, float t) const {
        const float cos = w * q.w + x * q.x + y * q.y + z * q.z;
        const float angle = std::acos(std::abs(cos));
        return                           (std::sin(angle * (1 - t)) * (*this) +
               (cos < 0 ? -1.0f : 1.0f) * std::sin(angle *      t)  * q     ) / std::sin(angle);
    }

    Quaternion Quaternion::slerp(const Quaternion& q, float t, int revolutions) const {
        const float cos = w * q.w + x * q.x + y * q.y + z * q.z;
        const float angle = std::acos(std::abs(cos)), rotation = angle + (float)revolutions * PI;
        return                           (std::sin(angle - t * rotation) * (*this) +
               (cos < 0 ? -1.0f : 1.0f) * std::sin(        t * rotation) * q     ) / std::sin(angle);
    }

    Quaternion Quaternion::operator*(float v) const { return { w * v, x * v, y * v, z * v }; }
    Quaternion Quaternion::operator/(float v) const { return operator*(1.0f / v); }
    Quaternion Quaternion::operator*(const Quaternion& q) const {
        return { w * q.w - xyz().dot(q.xyz()), w * q.xyz() + q.w * xyz() + xyz().cross(q.xyz()) };
    }
    Quaternion Quaternion::operator/(const Quaternion& q) const { return operator*(q.inv()); }

    Quaternion& Quaternion::operator*=(const Quaternion& q) { return *this = operator*(q); }
    Quaternion& Quaternion::operator/=(const Quaternion& q) { return *this = operator/(q); }

    Quaternion Quaternion::then(const Quaternion& q) const { return q * *this; }
    Quaternion& Quaternion::rotate_by(const Quaternion& q) { return *this = then(q); }
    fVector3 Quaternion::rotate(const fVector3& v) const { return ((*this) * v * conj()).xyz(); }
    fVector3 Quaternion::invrotate(const fVector3& v) const { return (conj() * v * (*this)).xyz(); }

    Quaternion operator+(float w, const Quaternion& q) { return  q + w; }
    Quaternion operator-(float w, const Quaternion& q) { return -q + w; }
    Quaternion operator*(float w, const Quaternion& q) { return  q * w; }
    Quaternion operator/(float w, const Quaternion& q) { return q.inv() * w; }
} // Quasi
