#pragma once
#include "Quaternion.h"

namespace Quasi::Math {
    struct Rotation3D : private Quaternion {
        Rotation3D() = default;
        Rotation3D(Radians x, Radians y, Radians z) : Quaternion(FromEulerAngles({ x, y, z })) {}
        Rotation3D(const Rotation2D& x, const Rotation2D& y, const Rotation2D& z) : Quaternion(FromEulerAngles({ x, y, z })) {}
    private:
        Rotation3D(const Quaternion& q) : Quaternion(q) {}
    public:
        static Rotation3D FromQuat(const Quaternion& c) { return { c }; }
        Quaternion&       AsQuat()       { return static_cast<Quaternion&>(*this); }
        const Quaternion& AsQuat() const { return static_cast<const Quaternion&>(*this); }

        static Rotation3D RotateAxis(const fv3& axis, const Rotation2D& r) { return Quaternion::RotateAxis(axis, r); }
        static Rotation3D LookAt(const fv3& direction, const fv3& front) { return Quaternion::LookAt(direction, front); }
        static Rotation3D RotateX(const Rotation2D& r) { return Quaternion::RotateX(r); }
        static Rotation3D RotateY(const Rotation2D& r) { return Quaternion::RotateY(r); }
        static Rotation3D RotateZ(const Rotation2D& r) { return Quaternion::RotateZ(r); }

        fv3 IHat() const;
        fv3 JHat() const;
        fv3 KHat() const;

        Vec3<Radians> EulerAngles() const { return ToEulerAngles(); }
        Radians AngleBetween(const Rotation3D& r) const;

        Rotation3D RotateBy   (const Rotation3D& r) const;
        Rotation3D RotateByInv(const Rotation3D& r) const;

        Rotation3D Halved() const;
        Rotation3D Mul(f32 p) const { return Pow(p); }
        Rotation3D Doubled()  const;
        Rotation3D Tripled()  const;
        Rotation3D Inverse()  const { return Conj(); }

        Matrix3x3 AsMatrixLinear() const { return AsQuat().AsMatrixLinear(); }
        Matrix3D  AsMatrix()       const { return AsQuat().AsMatrix(); }
        fv3 Rotate   (const fv3& v) const;
        fv3 InvRotate(const fv3& v) const;

        Rotation3D Lerp(const Rotation3D& z, f32 t) const { return AsQuat().Slerp(z, t); }
        Rotation3D Lerp(const Rotation3D& z, f32 t, int revolutions) const { return AsQuat().Slerp(z, t, revolutions); }

        Rotation3D operator+() const { return *this; }
        Rotation3D operator-() const { return Inverse(); }
        Rotation3D operator+(const Rotation3D& r) const { return RotateBy(r); }
        Rotation3D operator-(const Rotation3D& r) const { return RotateByInv(r); }
        Rotation3D operator*(f32 mul) const { return Mul(mul); }
        Rotation3D operator/(f32 div) const { return Mul(1.0f / div); }
        Rotation3D& operator+=(const Rotation3D& r) { return *this = RotateBy(r); }
        Rotation3D& operator-=(const Rotation3D& r) { return *this = RotateByInv(r); }
        Rotation3D& operator*=(f32 mul) { return *this = Mul(mul); }
        Rotation3D& operator/=(f32 div) { return *this = Mul(1.0f / div); }
        fv3 operator*(const fv3& v) const { return Rotate(v); }

        static Rotation3D Random(RandomGenerator& rg);
    };

    struct MatrixTransform3D {
        Matrix3D transform, normalMatrix;

        fv3 Transform(const fv3& p) const { return (fv3)(transform * p); }
        fv3 TransformNormal(const fv3& n) const { return ((fv3)(normalMatrix * n)).Norm(); }
    };

    template <class Tf> concept ITransformer3D = requires (const Tf& t, const fv3& vec) {
        { t.Transform(vec) }       -> ConvTo<fv3>;
        { t.TransformNormal(vec) } -> ConvTo<fv3>;
    };

    template <class T>
    struct InverseTransform3D {
        static_assert(ITransformer3D<T>, "T should be a transformer3d"); // delayed constraint
        const T& tformer;
        fv3 Transform             (const fv3& point)  const { return tformer.TransformInverse(point); }
        fv3 TransformInverse      (const fv3& point)  const { return tformer.Transform(point); }
        fv3 TransformNormal       (const fv3& normal) const { return tformer.TransformInverseNormal(normal); }
        fv3 TransformInverseNormal(const fv3& normal) const { return tformer.TransformNormal(normal); }
    };

    struct Transform3D {
        fv3 position = 0, scale = 1;
        Rotation3D rotation {};

        Transform3D() = default;
        Transform3D(const fv3& pos, const fv3& scale = 1, const Rotation3D& rot = {})
            : position(pos), scale(scale), rotation(rot) {}

        static Transform3D Translate(const fv3& p);
        static Transform3D Scale(const fv3& s);
        static Transform3D Rotation(const Rotation3D& r);

        Transform3D NormalTransform() const;

        fv3 Transform (const fv3& point) const;
        fv3 TransformInverse (const fv3& point) const;
        fv3 TransformNormal (const fv3& normal) const;
        fv3 TransformInverseNormal (const fv3& normal) const;

        InverseTransform3D<Transform3D> Inverse() const { return { *this }; }

        Transform3D Applied(const Transform3D& transformer) const; // apply transform onto self
        Transform3D AppliedTo(const Transform3D& transformed) const { return transformed.Applied(*this); }
        Transform3D& Apply(const Transform3D& transformer); // apply transform onto self
        void ApplyTo(Transform3D& transformed) const { transformed.Apply(*this); }

        Transform3D Then(const Transform3D& t) const { return AppliedTo(t); }

        void Reset();

        Matrix3x3 LinearMatrix() const;
        Matrix3D  TransformMatrix() const;

        fv3 operator*(const fv3& p) const { return Transform(p); }
        Transform3D operator*(const Transform3D& t) const { return Applied(t); }
        Transform3D& operator*=(const Transform3D& t) { return Apply(t); }
    };
} // Quasi
