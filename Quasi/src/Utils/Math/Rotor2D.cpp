#include "Rotor2D.h"
#include "Random.h"

namespace Quasi::Math {
    f32 Rotor2D::Angle() const { return std::atan2(im, re); }
    f32 Rotor2D::AngleBetween(const Rotor2D& other) const { return std::acos(re * other.re + im * other.im); }

    Rotor2D Rotor2D::RotateCCW90() const { return fComplex { -im,  re }; }
    Rotor2D Rotor2D::RotateCW90()  const { return fComplex {  im, -re }; }
    Rotor2D Rotor2D::Rotate180()   const { return fComplex { -re, -im }; }
    Rotor2D Rotor2D::RotateBy(f32 theta) const { return RotateBy(Rotor2D { theta }); }
    Rotor2D Rotor2D::RotateBy   (const Rotor2D& r) const { return fComplex { re * r.re - im * r.im, re * r.im + im * r.re }; }
    Rotor2D Rotor2D::RotateByInv(const Rotor2D& r) const { return fComplex { re * r.re + im * r.im, im * r.re - re * r.im }; }

    Rotor2D Rotor2D::Halved() const { return UnitSqrt(); }
    Rotor2D Rotor2D::HalvedCCW() const { return UnitUpperSqrt(); }
    Rotor2D Rotor2D::Mul(f32 p) const { return ExpImag(Angle() * p); }

    fv2 Rotor2D::Rotate   (const fv2& v) const { return { v.x * re - v.y * im, v.x * im + v.y * re }; }
    fv2 Rotor2D::InvRotate(const fv2& v) const { return { v.x * re + v.y * im, v.y * re - v.x * im }; }

    Rotor2D Rotor2D::Lerp(const Rotor2D& z, f32 t) const {
        const f32 theta = AngleBetween(z);
        const f32 inv = 1 / std::sin(theta), p = std::sin(theta * (1 - t)) * inv, q = std::sin(theta * t) * inv;
        return AsComplex() * p + z.AsComplex() * q;
    }

    Rotor2D Rotor2D::Random(RandomGenerator& rg) {
        return rg.Get<f32>(0, TAU);
    }
}