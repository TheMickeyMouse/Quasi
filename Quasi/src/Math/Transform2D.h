#pragma once
#include "Complex.h"
#include "Constants.h"

namespace Quasi::Math {
    struct MatrixTransform2D {
        Matrix2D transform, normalMatrix;

        fVector2 Transform(const fVector2& p) const { return transform * p; }
        fVector2 TransformNormal(const fVector2& n) const { return (normalMatrix * n).norm(); }
    };

    template <class T> concept ITransformer2D = requires (const T& t, const fVector2& vec) {
        { t.Transform(vec) } -> std::convertible_to<fVector2>;
        { t.TransformNormal(vec) } -> std::convertible_to<fVector2>;
    };

    template <class T>
    struct InverseTransform2D {
        static_assert(ITransformer2D<T>, "T should be a transformer2d"); // delayed constraint
        const T& tformer;
        fVector2 Transform       (const fVector2& point) const { return tformer.TransformInverse(point); }
                      void     TransformInplace(fVector2&       point) const { tformer.TransformInverseInplace(point); }
        fVector2 TransformInverse       (const fVector2& point) const { return tformer.Transform(point); }
                      void     TransformInverseInplace(fVector2&       point) const { tformer.TransformInplace(point); }
        fVector2 TransformNormal       (const fVector2& normal) const { return tformer.TransformInverseNormal(normal); }
                      void     TransformNormalInplace(fVector2&       normal) const { tformer.TransformInverseNormalInplace(normal); }
        fVector2 TransformInverseNormal       (const fVector2& normal) const { return tformer.TransformNormal(normal); }
                      void     TransformInverseNormalInplace(fVector2&       normal) const { tformer.TransformNormalInplace(normal); }
    };

    struct Transform2D {
        fVector2 position = 0, scale = 1;
        fComplex rotation = 1;

        Transform2D() = default;
        Transform2D(const fVector2& pos, const fVector2& scale, const fComplex& rotation)
            : position(pos), scale(scale), rotation(rotation) {}
        Transform2D(const fVector2& pos, const fVector2& scale = 1, float rAngle = 0.0f)
            : Transform2D(pos, scale, fComplex::rotate(rAngle)) {}

        template <class Cmplx>
        struct RotationAngleProxy {
            Cmplx& r;

            operator float() const { return r.angle(); }
            RotationAngleProxy& operator=(float angle) requires (!std::is_const_v<Cmplx>)
            { r = fComplex::rotate(angle); return *this; }

            float Radians() const { return r.angle(); }
            float Degrees() const { return Radians() * RAD2DEG; }

            const Cmplx& AsComplex() const { return r; }
            Cmplx& AsComplex() { return r; }
        };

        RotationAngleProxy<const fComplex> RotationAngle() const { return { rotation }; }
        RotationAngleProxy<fComplex> RotationAngle() { return { rotation }; }

        void Translate(const fVector2& p);
        void Scale(const fVector2& s);
        void Rotate(float r);
        void Rotate(const fComplex& r);
        Transform2D Translated(const fVector2& p) const;
        Transform2D Scaled(const fVector2& s) const;
        Transform2D Rotated(float r) const;
        Transform2D Rotated(const fComplex& r) const;

        static Transform2D Translation(const fVector2& p);
        static Transform2D Scaling(const fVector2& s);
        static Transform2D Rotation(float r);
        static Transform2D Rotation(const fComplex& q);

        Transform2D NormalTransform() const;


        fVector2 Transform       (const fVector2& point) const;
                      void     TransformInplace(fVector2&       point) const;
        fVector2 TransformInverse       (const fVector2& point) const;
                      void     TransformInverseInplace(fVector2&       point) const;
        fVector2 TransformNormal       (const fVector2& normal) const;
                      void     TransformNormalInplace(fVector2&       normal) const;
        fVector2 TransformInverseNormal       (const fVector2& normal) const;
                      void     TransformInverseNormalInplace(fVector2&       normal) const;

        InverseTransform2D<Transform2D> Inverse() const { return { *this }; }

        Transform2D Applied(const Transform2D& transformer) const; // apply transform onto self
        Transform2D AppliedTo(const Transform2D& transformed) const { return transformed.Applied(*this); }
        Transform2D& Apply(const Transform2D& transformer); // apply transform onto self
        void ApplyTo(Transform2D& transformed) const { transformed.Apply(*this); }

        Transform2D Then(const Transform2D& t) const { return AppliedTo(t); }

        Transform3D As3D() const;

        void Reset();

        Matrix2x2 LinearMatrix() const;
        Matrix2D  TransformMatrix() const;

        fVector2 operator*(const fVector2& p) const { return Transform(p); }
        Transform2D operator*(const Transform2D& t) const { return Applied(t); }
        Transform2D& operator*=(const Transform2D& t) { return Apply(t); }
    };

    template <class T> VectorN<2, T> VectorN<2, T>::transformed_by(const Transform2D& transform) const {
        return transform.Transform(*this);
    }
    template <class T> VectorN<2, T>& VectorN<2, T>::transform_by(const Transform2D& transform) {
        transform.TransformInplace(*this);
        return *this;
    }
} // Quasi
