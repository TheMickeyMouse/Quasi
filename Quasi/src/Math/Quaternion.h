#pragma once
#include "Matrix.h"

namespace Quasi::Math {
    template <class> struct Complex;
    using fComplex = Complex<float>;

    struct Quaternion {
        float w, x, y, z;

        Quaternion(float w = 1) : w(w), x(0), y(0), z(0) {}
    private:
        Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
        Quaternion(float w, const fVector3& xyz) : w(w), x(xyz.x), y(xyz.y), z(xyz.z) {}
        Quaternion(float x, float y, float z) : w(0), x(x), y(y), z(z) {}
        Quaternion(const fVector3& xyz) : Quaternion(xyz.x, xyz.y, xyz.z) {}
    public:
        fVector3 xyz()  const { return { x, y, z }; }
        fVector4 wxyz() const { return { w, x, y, z }; }

        static constexpr Str ROTATION_ORDER = "ZXY"; // readonly property thats not used

        static Quaternion from_vec(const fVector4& q) { return { q.w, q.x, q.y, q.z }; }
        static Quaternion from_vec(const fVector3& q) { return { 0,   q.x, q.y, q.z }; }
        static Quaternion from_wxyz(float w, float x, float y, float z) { return { w, x, y, z }; }
        static const Quaternion i;
        static const Quaternion j;
        static const Quaternion k;

        static Quaternion rotate_axis(const fVector3& axis, float rotation);
        static Quaternion rotate_axis(const fVector3& axis, fComplex rotation);
        static Quaternion rotate_x(float xrot);
        static Quaternion rotate_y(float yrot);
        static Quaternion rotate_z(float zrot);
        static Quaternion rotate_xyz(const fVector3& rotation);
        static Quaternion rotate_to(const fVector3& from, const fVector3& to); // both vectors assumed to be normed
        fVector3 xyzrot() const;

        static Quaternion from_euler(const fVector3& rotation) { return rotate_xyz(rotation); }
        fVector3 to_euler() const { return xyzrot(); }

        static Quaternion look_at(const fVector3& fwd, const fVector3& worldFront); // both vectors assumed to be normed

        Matrix3x3 as_matrix() const;
        Matrix3D  as_rotation_matrix() const;
        Matrix4x4 as_compute_matrix() const;

        float lensq() const;
        float len() const;
        float abs() const;
        float distsq(const Quaternion& q) const;
        float dist(const Quaternion& q) const;
        Quaternion norm() const;

        Quaternion exp() const;
        Quaternion log() const;
        Quaternion pow(float p) const;

        Quaternion operator+() const { return *this; }
        Quaternion operator-() const { return { -w, -x, -y, -z }; }
        Quaternion conj() const;
        Quaternion inv() const;

        Quaternion muli() const;
        Quaternion mulj() const;
        Quaternion mulk() const;

        Quaternion lerp(const Quaternion& q, float t) const;
        Quaternion slerp(const Quaternion& q, float t) const;
        Quaternion slerp(const Quaternion& q, float t, int revolutions) const;

        Quaternion operator+(const Quaternion& q) const;
        Quaternion operator-(const Quaternion& q) const;
        Quaternion operator*(float v) const;
        Quaternion operator/(float v) const;
        Quaternion operator*(const Quaternion& q) const;
        Quaternion operator/(const Quaternion& q) const;
        Quaternion& operator+=(const Quaternion& q);
        Quaternion& operator-=(const Quaternion& q);
        Quaternion& operator*=(float v);
        Quaternion& operator*=(const Quaternion& q);
        Quaternion& operator/=(float v);
        Quaternion& operator/=(const Quaternion& q);

        Quaternion then(const Quaternion& q) const;
        Quaternion& rotate_by(const Quaternion& q);
        fVector3 rotate(const fVector3& v) const;
        fVector3 invrotate(const fVector3& v) const;

        bool operator==(const Quaternion&) const = default;

        static Quaternion random_rot(RandomGenerator& rg);
    };

    Quaternion operator+(float w, const Quaternion& q);
    Quaternion operator-(float w, const Quaternion& q);
    Quaternion operator*(float w, const Quaternion& q);
    Quaternion operator/(float w, const Quaternion& q);

    template <class T>
    typename VectorN<3, T>::float_vec VectorN<3, T>::rotated_by(const Quaternion& rotation) const {
        return rotation.rotate(*this);
    }

    template <class T>
    VectorN<3, T>& VectorN<3, T>::rotate_by(const Quaternion& rotation) requires traits_float {
        return *this = rotation.rotate(*this);
    }

    inline Quaternion operator""_qi(long double i) { return Quaternion::from_wxyz(0, (float)i, 0, 0); }
    inline Quaternion operator""_qj(long double j) { return Quaternion::from_wxyz(0, 0, (float)j, 0); }
    inline Quaternion operator""_qk(long double k) { return Quaternion::from_wxyz(0, 0, 0, (float)k); }
} // Quasi

namespace Quasi::Text {
    template <>
    struct Formatter<Math::Quaternion> {
        static usize FormatTo(StringWriter sw, const Math::Quaternion& q, Str) {
            return Text::FormatTo(sw,
                "{} {} {}i {} {}j {} {}k",
                q.w,
                q.x < 0 ? '-' : '+', q.x,
                q.y < 0 ? '-' : '+', q.y,
                q.z < 0 ? '-' : '+', q.z
            );
        }
    };
}
