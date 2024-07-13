#pragma once
#include "Complex.h"
#include "Constants.h"

namespace Quasi::Math {
    struct Transform2D {
        fVector2 position = 0, scale = 1;
        fComplex rotation = 1;

        Transform2D() = default;
        Transform2D(fVector2 pos, fVector2 scale, fComplex rotation)
            : position(pos), scale(scale), rotation(rotation) {}
        Transform2D(fVector2 pos, fVector2 scale = 1, float rAngle = 0.0f)
            : Transform2D(pos, scale, fComplex::rotate(rAngle)) {}

        template <class Cmplx>
        struct RotationAngleProxy {
            Cmplx& r;

            operator float() const { return r.angle(); }
            RotationAngleProxy& operator=(float angle) requires (!std::is_const_v<Cmplx>)
            { r = fComplex::rotate(angle); return *this; }

            [[nodiscard]] float Radians() const { return r.angle(); }
            [[nodiscard]] float Degrees() const { return Radians() * RAD2DEG; }

            [[nodiscard]] const Cmplx& AsComplex() const { return r; }
            Cmplx& AsComplex() { return r; }
        };

        [[nodiscard]] RotationAngleProxy<const fComplex> RotationAngle() const { return { rotation }; }
        RotationAngleProxy<fComplex> RotationAngle() { return { rotation }; }

        void Translate(fVector2 p);
        void Scale(fVector2 s);
        void Rotate(float r);
        void Rotate(fComplex r);
        [[nodiscard]] Transform2D Translated(fVector2 p) const;
        [[nodiscard]] Transform2D Scaled(fVector2 s) const;
        [[nodiscard]] Transform2D Rotated(float r) const;
        [[nodiscard]] Transform2D Rotated(fComplex r) const;

        static Transform2D Translation(const fVector2& p);
        static Transform2D Scaling(const fVector2& s);
        static Transform2D Rotation(float r);
        static Transform2D Rotation(const fComplex& q);

        [[nodiscard]] Transform2D Inverse() const;
        [[nodiscard]] Transform2D NormalTransform() const;

        [[nodiscard]] fVector2 Transform(fVector2 point) const;
        void TransformInplace(fVector2& point) const;
        [[nodiscard]] fVector2 TransformNormal(fVector2 normal) const;
        void TransformNormalInplace(fVector2& normal) const;
        [[nodiscard]] Transform2D Applied(const Transform2D& transformer) const; // apply transform onto self
        [[nodiscard]] Transform2D AppliedTo(const Transform2D& transformed) const { return transformed.Applied(*this); }
        Transform2D& Apply(const Transform2D& transformer); // apply transform onto self
        void ApplyTo(Transform2D& transformed) const { transformed.Apply(*this); }

        [[nodiscard]] Transform2D Then(const Transform2D& t) const { return AppliedTo(t); }

        void Reset();

        [[nodiscard]] Matrix2x2 LinearMatrix() const;
        [[nodiscard]] Matrix2D  TransformMatrix() const;

        [[nodiscard]] fVector2 operator*(fVector2 p) const { return Transform(p); }
        [[nodiscard]] Transform2D operator*(const Transform2D& t) const { return Applied(t); }
        Transform2D& operator*=(const Transform2D& t) { return Apply(t); }
    };

    template <class T> VectorN<2, T> VectorN<2, T>::transformed_by(const Transform2D& transform) const {
        return transform.Transform(*this);
    }
    template <class T> VectorN<2, T>& VectorN<2, T>::transform_by(const Transform2D& transform) {
        transform.TransformInplace(*this);
        return *this;
    }

    struct MatrixTransform2D {
        Matrix2D transform, normalMatrix;

        [[nodiscard]] fVector2 Transform(fVector2 p) const { return transform * p; }
        [[nodiscard]] fVector2 TransformNormal(fVector2 n) const { return (normalMatrix * n).norm(); }
    };

    template <class T> concept ITransformer2D = requires (const T& t, fVector2 vec) {
        { t.Transform(vec) } -> std::convertible_to<fVector2>;
        { t.TransformNormal(vec) } -> std::convertible_to<fVector2>;
    };
} // Quasi
