#pragma once
#include "Complex.h"

namespace Quasi::Math {
    struct Rotation2D : private fComplex {
        Rotation2D() : fComplex(1, 0) {}
        Rotation2D(Radians theta) : fComplex(Cos(theta), Sin(theta)) {}
    private:
        Rotation2D(const fComplex& c) : fComplex(c) {}
    public:
        static Rotation2D FromComplex(const fComplex& c) { return { c }; }
        fComplex&       AsComplex()       { return static_cast<fComplex&>(*this); }
        const fComplex& AsComplex() const { return static_cast<const fComplex&>(*this); }

        fv2 IHat() const { return {  re, im }; }
        fv2 JHat() const { return { -im, re }; }

        Radians Angle() const;
        Radians AngleBetween(const Rotation2D& other) const;

        Rotation2D RotateCCW90() const;
        Rotation2D RotateCW90()  const;
        Rotation2D Rotate180()   const;
        Rotation2D RotateBy(Radians theta) const;
        Rotation2D RotateBy   (const Rotation2D& r) const;
        Rotation2D RotateByInv(const Rotation2D& r) const;

        Rotation2D Halved() const;
        Rotation2D Mul(f32 p) const;
        Rotation2D Doubled()  const { return Squared(); }
        Rotation2D Tripled()  const { return Cubed(); }
        Rotation2D Inverse()  const { return Conj(); }

        Matrix2x2 AsMatrixLinear() const { return AsComplex().AsMatrixLinear(); }
        Matrix2D  AsMatrix()       const { return AsComplex().AsMatrix(); }
        fv2 Rotate   (const fv2& v) const;
        fv2 InvRotate(const fv2& v) const;

        Rotation2D Lerp(const Rotation2D& z, f32 t) const;

        Rotation2D operator+() const { return *this; }
        Rotation2D operator-() const { return Inverse(); }
        Rotation2D operator+(const Rotation2D& r) const { return RotateBy(r); }
        Rotation2D operator-(const Rotation2D& r) const { return RotateByInv(r); }
        Rotation2D operator*(f32 mul) const { return Mul(mul); }
        Rotation2D operator/(f32 div) const { return Mul(1.0f / div); }
        Rotation2D& operator+=(const Rotation2D& r) { return *this = RotateBy(r); }
        Rotation2D& operator-=(const Rotation2D& r) { return *this = RotateByInv(r); }
        Rotation2D& operator*=(f32 mul) { return *this = Mul(mul); }
        Rotation2D& operator/=(f32 div) { return *this = Mul(1.0f / div); }

        static Rotation2D Random(RandomGenerator& rg);
    };

    struct MatrixTransform2D {
        Matrix2D transform, normalMatrix;

        fv2 Transform(const fv2& p) const { return (fv2)(transform * p); }
        fv2 TransformNormal(const fv2& n) const { return ((fv2)(normalMatrix * n)).Norm(); }
    };

    template <class TF> concept ITransformer2D = requires (const TF& t, const fv2& vec) {
        { t.Transform(vec) }       -> ConvTo<fv2>;
        { t.TransformNormal(vec) } -> ConvTo<fv2>;
    };

    template <class T>
    struct InverseTransform2D {
        static_assert(ITransformer2D<T>, "T should be a transformer2d"); // delayed constraint
        const T& tformer;
        fv2 Transform(const fv2& point)               const { return tformer.TransformInverse(point); }
        fv2 TransformInverse(const fv2& point)        const { return tformer.Transform(point); }
        fv2 TransformNormal (const fv2& normal)       const { return tformer.TransformInverseNormal(normal); }
        fv2 TransformInverseNormal(const fv2& normal) const { return tformer.TransformNormal(normal); }
    };

    struct Transform3D;

    struct Transform2D {
        fv2 position = 0, scale = 1;
        Rotation2D rotation;

        Transform2D() = default;
        Transform2D(const fv2& pos, const fv2& scale = 1, const Rotation2D& rotation = {})
            : position(pos), scale(scale), rotation(rotation) {}

        static Transform2D Translate(const fv2& p);
        static Transform2D Scale(const fv2& s);
        static Transform2D Rotation(const Rotation2D& r);

        Transform2D NormalTransform() const;

        fv2 Transform             (const fv2& point) const;
        fv2 TransformInverse      (const fv2& point) const;
        fv2 TransformNormal       (const fv2& normal) const;
        fv2 TransformInverseNormal(const fv2& normal) const;

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

        fv2 operator*(const fv2& p) const { return Transform(p); }
        Transform2D operator*(const Transform2D& t) const { return Applied(t); }
        Transform2D& operator*=(const Transform2D& t) { return Apply(t); }
    };

    template <Numeric T> Vector<T, 2> Vector<T, 2>::RotateBy(const Rotation2D& r) const {
        return r.Rotate(*this);
    }
    template <Numeric T> Vector<T, 2> Vector<T, 2>::TransformBy(const Transform2D& t) const {
        return t.Transform(*this);
    }
} // Quasi
