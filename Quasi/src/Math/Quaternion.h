#pragma once
#include "Matrix.h"

namespace Quasi::Math {
    struct Quaternion {
        float w, x, y, z;

        Quaternion(float w) : w(w), x(0), y(0), z(0) {}
    private:
        Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
        Quaternion(float w, const fVector3& xyz) : w(w), x(xyz.x), y(xyz.y), z(xyz.z) {}
        Quaternion(float x, float y, float z) : w(0), x(x), y(y), z(z) {}
        Quaternion(const fVector3& xyz) : Quaternion(xyz.x, xyz.y, xyz.z) {}
    public:
        [[nodiscard]] fVector3 xyz()  const { return { x, y, z }; }
        [[nodiscard]] fVector4 wxyz() const { return { w, x, y, z }; }

        static constexpr Str ROTATION_ORDER = "ZXY"; // readonly property thats not used

        static Quaternion from_vec(const fVector4& q) { return { q.w, q.x, q.y, q.z }; }
        static Quaternion from_vec(const fVector3& q) { return { 0,   q.x, q.y, q.z }; }
        static const Quaternion i;
        static const Quaternion j;
        static const Quaternion k;

        static Quaternion rotate_axis(const fVector3& axis, float rotation);
        static Quaternion rotate_x(float xrot);
        static Quaternion rotate_y(float yrot);
        static Quaternion rotate_z(float zrot);
        static Quaternion rotate_xyz(const fVector3& rotation);
        [[nodiscard]] fVector3 xyzrot() const;

        static Quaternion from_euler(const fVector3& rotation) { return rotate_xyz(rotation); }
        [[nodiscard]] fVector3 to_euler() const { return xyzrot(); }

        [[nodiscard]] Matrix3x3 as_matrix() const;
        [[nodiscard]] Matrix3D  as_rotation_matrix() const;
        [[nodiscard]] Matrix4x4 as_compute_matrix() const;

        [[nodiscard]] String str() const;

        [[nodiscard]] float lensq() const;
        [[nodiscard]] float len() const;
        [[nodiscard]] float abs() const;
        [[nodiscard]] float distsq(const Quaternion& q) const;
        [[nodiscard]] float dist(const Quaternion& q) const;
        [[nodiscard]] Quaternion norm() const;

        [[nodiscard]] Quaternion exp() const;
        [[nodiscard]] Quaternion log() const;

        [[nodiscard]] Quaternion operator+() const { return *this; }
        [[nodiscard]] Quaternion operator-() const { return { -w, -x, -y, -z }; }
        [[nodiscard]] Quaternion conj() const;
        [[nodiscard]] Quaternion inv() const;

        [[nodiscard]] Quaternion muli() const;
        [[nodiscard]] Quaternion mulj() const;
        [[nodiscard]] Quaternion mulk() const;

        [[nodiscard]] Quaternion lerp(const Quaternion& q, float t) const;
        [[nodiscard]] Quaternion slerp(const Quaternion& q, float t) const;
        [[nodiscard]] Quaternion slerp(const Quaternion& q, float t, int revolutions) const;

        [[nodiscard]] Quaternion operator+(const Quaternion& q) const;
        [[nodiscard]] Quaternion operator-(const Quaternion& q) const;
        [[nodiscard]] Quaternion operator*(float v) const;
        [[nodiscard]] Quaternion operator/(float v) const;
        [[nodiscard]] Quaternion operator*(const Quaternion& q) const;
        [[nodiscard]] Quaternion operator/(const Quaternion& q) const;
        [[nodiscard]] Quaternion& operator+=(const Quaternion& q);
        [[nodiscard]] Quaternion& operator-=(const Quaternion& q);
        [[nodiscard]] Quaternion& operator*=(float v);
        [[nodiscard]] Quaternion& operator*=(const Quaternion& q);
        [[nodiscard]] Quaternion& operator/=(float v);
        [[nodiscard]] Quaternion& operator/=(const Quaternion& q);

        [[nodiscard]] Quaternion then(const Quaternion& q) const;
        [[nodiscard]] fVector3 rotate(const fVector3& v) const;

        bool operator==(const Quaternion&) const = default;

        friend Quaternion operator""qi(long double);
        friend Quaternion operator""qj(long double);
        friend Quaternion operator""qk(long double);
    };

    const Quaternion Quaternion::i = { 1, 0, 0 };
    const Quaternion Quaternion::j = { 0, 1, 0 };
    const Quaternion Quaternion::k = { 0, 0, 1 };

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

    Quaternion operator""qi(long double i);
    Quaternion operator""qj(long double j);
    Quaternion operator""qk(long double k);
} // Quasi
