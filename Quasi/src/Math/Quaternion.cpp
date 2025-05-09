#include "Quaternion.h"

#include "Complex.h"
#include "Constants.h"
#include "Transform2D.h"

namespace Quasi::Math {
    Quaternion Quaternion::RotateAxis(const fv3& axis, const Rotation2D& rotation) {
        const auto [cos, sin] = rotation.Halved().IHat();
        return { cos, axis * sin };
    }

    Quaternion Quaternion::RotateX(const Rotation2D& r) {
        const auto [cos, sin] = r.Halved().IHat();
        return { cos, sin, 0, 0 };
    }
    Quaternion Quaternion::RotateY(const Rotation2D& r) {
        const auto [cos, sin] = r.Halved().IHat();
        return { cos, 0, sin, 0 };
    }
    Quaternion Quaternion::RotateZ(const Rotation2D& r) {
        const auto [cos, sin] = r.Halved().IHat();
        return { cos, 0, 0, sin };
    }
    Quaternion Quaternion::RotateXYZ(const Vec3<Rotation2D>& r) { return RotateY(r.y) * RotateX(r.x) * RotateZ(r.z); }
    Quaternion Quaternion::RotateTo(const fv3& from, const fv3& to) {
        return { from.Dot(to), from.Cross(to) };
    }

    Vec3<Radians> Quaternion::ToEulerAngles() const {
        const f32 a = w - x, b = z + y, c = x + w, d = y - z;
        const Radians xrot = Arcsin(2 * (a * a + b * b) / (a * a + b * b + c * c + d * d) - 1);
        const Radians posAngle = Atan2(b, a), negAngle = Atan2(d, c);
        Radians yrot = posAngle + negAngle;
        Radians zrot = posAngle - negAngle;
        yrot += Radians((*yrot < -PI ? TAU : 0.0f) + (*yrot > +PI ? -TAU : 0.0f));
        zrot += Radians((*zrot < -PI ? TAU : 0.0f) + (*zrot > +PI ? -TAU : 0.0f));
        return { -xrot, yrot, zrot };
    }

    Quaternion Quaternion::LookAt(const fv3& direction, const fv3& worldFront) {
        return RotateTo(worldFront, direction);
    }

    Matrix3x3 Quaternion::AsMatrixLinear() const {
        const float xx = x * x, yy = y * y, zz = z * z,
                    xy = x * y, zw = z * w, xz = x * z, yw = y * w, xw = x * w, yz = y * z;
        return {{ 1 - 2 * (yy + zz),     2 * (xy - zw),     2 * (xz + yw),
                      2 * (xy + zw), 1 - 2 * (xx + zz),     2 * (yz - xw),
                      2 * (xz - yw),     2 * (yz + xw), 1 - 2 * (xx + yy) }};
    }

    Matrix3D Quaternion::AsMatrix() const {
        const float xx = x * x, yy = y * y, zz = z * z,
                    xy = x * y, zw = z * w, xz = x * z, yw = y * w, xw = x * w, yz = y * z;
        return {{
            1 - 2 * (yy + zz),     2 * (xy - zw),     2 * (xz + yw), 0,
                2 * (xy + zw), 1 - 2 * (xx + zz),     2 * (yz - xw), 0,
                2 * (xz - yw),     2 * (yz + xw), 1 - 2 * (xx + yy), 0,
                            0,                 0,                 0, 1
        }};
    }

    Matrix4x4 Quaternion::AsComputeMatrix() const {
        return {{
            w, -x, -y, -z,
            x,  w, -z,  y,
            y,  z,  w,  x,
            z, -y,  x,  w,
        }};
    }

    f32 Quaternion::LenSq() const { return w * w + x * x + y * y + z * z; }
    f32 Quaternion::Len()   const { return std::sqrt(LenSq()); }
    f32 Quaternion::Abs()   const { return Len(); }
    f32 Quaternion::DistSq(const Quaternion& q) const { return (*this - q).LenSq(); }
    f32 Quaternion::Dist  (const Quaternion& q) const { return (*this - q).Len(); }
    f32 Quaternion::Dot(const Quaternion& q) const { return w * q.w + x * q.x + y * q.y + z * q.z; }
    Quaternion Quaternion::Norm() const { return *this / Len(); }

    Quaternion Quaternion::Exp() const {
        const fv3 ijk = GetXYZ();
        const float r = ijk.Len(), e = std::exp(w);
        return { e * std::cos(r), r == 0 ? 0 : (ijk * (std::sin(r) / r)) };
    }

    Quaternion Quaternion::Log() const {
        const float r = Len();
        const fv3 v = GetXYZ();
        return { std::log(r), std::acos(w / r) * v.SafeNorm() };
    }

    Quaternion Quaternion::Pow(f32 p) const {
        return (Log() * p).Exp();
    }

    Quaternion Quaternion::operator+(const Quaternion& q) const { return { w + q.w, x + q.x, y + q.y, z + q.z }; }
    Quaternion Quaternion::operator-(const Quaternion& q) const { return { w - q.w, x - q.x, y - q.y, z - q.z }; }
    Quaternion& Quaternion::operator+=(const Quaternion& q) { w += q.w; x += q.x; y += q.y; z += q.z; return *this; }
    Quaternion& Quaternion::operator-=(const Quaternion& q) { w -= q.w; x -= q.x; y -= q.y; z -= q.z; return *this; }
    Quaternion& Quaternion::operator*=(float v) { w *= v; x *= v; y *= v; z *= v; return *this; }
    Quaternion& Quaternion::operator/=(float v) { w /= v; x /= v; y /= v; z /= v; return *this; }

    Quaternion Quaternion::Conj() const { return { w, -x, -y, -z }; }
    Quaternion Quaternion::Inv() const { return Conj() / LenSq(); }

    Quaternion Quaternion::MulI() const { return { -x, w, z, -y }; }
    Quaternion Quaternion::MulJ() const { return { -y, -z, w, x }; }
    Quaternion Quaternion::MulK() const { return { -z, y, -x, w }; }

    Quaternion Quaternion::Lerp(const Quaternion& q, f32 t) const {
        const f32 u = 1 - t;
        return { w * u + q.w * t, x * u + q.x * t, y * u + q.y * t, z * u + q.z * t };
    }

    Quaternion Quaternion::Slerp(const Quaternion& q, f32 t) const {
        const f32 cos = Dot(q);
        const f32 angle = std::acos(std::abs(cos));
        const f32 u = std::sin(angle * (1 - t)), v = (cos < 0 ? -1.0f : 1.0f) * std::sin(angle * t);
        return (u * (*this) + v * q) / std::sin(angle);
    }

    Quaternion Quaternion::Slerp(const Quaternion& q, float t, int revolutions) const {
        const f32 cos = Dot(q);
        const f32 angle = std::acos(std::abs(cos)), rotation = angle + PI * (f32)revolutions;
        const f32 u = std::sin(angle - t * rotation), v = (cos < 0 ? -1.0f : 1.0f) * std::sin(t * rotation);
        return (u * (*this) + v * q) / std::sin(angle);
    }

    Quaternion Quaternion::operator*(float v) const { return { w * v, x * v, y * v, z * v }; }
    Quaternion Quaternion::operator/(float v) const { return operator*(1.0f / v); }
    Quaternion Quaternion::operator*(const Quaternion& q) const {
        return { w * q.w - x * q.x - y * q.y - z * q.z,
                 w * q.x + q.w * x + y * q.z - z * q.y,
                 w * q.y + q.w * y + z * q.x - x * q.z,
                 w * q.z + q.w * z + x * q.y - y * q.x, };
    }
    Quaternion Quaternion::operator/(const Quaternion& q) const { return operator*(q.Inv()); }
    Quaternion& Quaternion::operator*=(const Quaternion& q) { return *this = operator*(q); }
    Quaternion& Quaternion::operator/=(const Quaternion& q) { return *this = operator/(q); }

    // fv3 Quaternion::rotate(const fv3& v) const { return ((*this) * v * conj()).xyz(); }
    // fv3 Quaternion::invrotate(const fv3& v) const { return (conj() * v * (*this)).xyz(); }

    Quaternion operator+(float w, const Quaternion& q) { return  q + w; }
    Quaternion operator-(float w, const Quaternion& q) { return -q + w; }
    Quaternion operator*(float w, const Quaternion& q) { return  q * w; }
    Quaternion operator/(float w, const Quaternion& q) { return  q.Inv() * w; }
} // Quasi
