#pragma once
#include "Matrix.h"

namespace Quasi::Math {
    template <class T>
    struct Complex {
        T re = 0, im = 0;
        using Flt = std::common_type_t<T, float>;

        Complex() = default;
        Complex(std::convertible_to<T> auto r) : re((T)r) {}
        Complex(std::convertible_to<T> auto r, std::convertible_to<T> auto i) : re((T)r), im((T)i) {}

        Vector2<T> as_vec() const { return { re, im }; }
        static Complex from_vec(const Vector2<T>& ri) { return { ri.x, ri.y }; }
        inline static const Complex i = { 0, 1 };

        Flt freal() const { return (Flt)re; }
        Flt fimag() const { return (Flt)im; }

        T   lensq() const { return re * re + im * im; }
        Flt len() const { return std::sqrt(lensq()); }
        Flt abs() const { return len(); }
        T   distsq(const Complex& z) const { return (*this - z).lensq(); }
        Flt dist(const Complex& z) const { return (*this - z).len(); }
        Complex<Flt> norm() const { return *this / len(); }

        Flt angle() const { return std::atan2(fimag(), freal()); }

        static Complex<Flt> expi(Flt imag) { return { std::cos(imag), std::sin(imag) }; }
        static Complex<Flt> rotate(Flt rotation) { return expi(rotation); }
        Complex<Flt> exp() const { return expi(fimag()) * std::exp(freal()); }
        Complex<Flt> log() const { return { std::log(lensq()) * 0.5f, angle() }; }

        Complex<Flt> sqrt() const {
            Flt r = abs(), imul = std::sqrt(r) / (*this + r).abs();
            return { (re + r) * imul, im * imul };
        }

        Complex<Flt> pow(float p) const {
            return std::pow(abs(), p) * expi(angle() * p);
        }

        Complex<Flt> pow(Complex p) const {
            return (log() * p).exp();
        }

        Complex powi(u32 p) const {
            Complex c = 1, base = *this;
            while (p) {
                if (p & 1) c *= base;
                p = p >> 1;
                base *= base;
            }
            return c;
        }

        Complex<Flt> asf() const { return { freal(), fimag() }; }

        Complex conj() const { return { re, -im }; }
        Complex<Flt> inv() const { Flt div = Flt(1) / lensq(); return { freal() * div, -fimag() * div }; }

        Matrix2x2 as_matrix() const {
            return { fVector2 { freal(), fimag() }, fVector2 { -fimag(), freal() } };
        }
        Matrix2D as_rotation_matrix() const {
            return { fVector3 { freal(), fimag(), 0 }, fVector3 { -fimag(), freal(), 0 }, fVector3 { 0, 0, 1 } };
        }
        Vector2<T> rotate(const Vector2<T>& v)    const { return { v.x * re - v.y * im, v.x * im + v.y * re }; }
        Vector2<T> invrotate(const Vector2<T>& v) const { return { v.x * re + v.y * im, v.y * re - v.x * im }; }

        Complex muli()    const { return { -im, re }; }
        Complex mulnegi() const { return { im, -re }; }

        Complex<Flt> lerp(const Complex& z, Flt t) const {
            return { freal() + (z.freal() - freal()) * t, fimag() + (z.fimag() - fimag()) * t };
        }

        Complex operator+() const { return *this; }
        Complex operator-() const { return (T)0 - *this; }
        Complex operator+(const Complex& z) const { return { re + z.re, im + z.im }; }
        Complex operator-(const Complex& z) const { return { re - z.re, im - z.im }; }
        Complex operator*(const Complex& z) const { return { re * z.re - im * z.im, re * z.im + im * z.re }; }
        Complex<Flt> operator/(const Complex& z) const { return asf() * z.inv(); }
        Complex& operator+=(const Complex& z) { re += z.re; im += z.im; return *this; }
        Complex& operator-=(const Complex& z) { re -= z.re; im -= z.im; return *this; }
        Complex& operator*=(const Complex& z) { T real = re; re = re * z.re - im * z.im; im = real * z.im + im * z.re; return *this; }
        Complex& operator/=(const Complex& z) { return (*this) *= z.inv(); }

        Complex operator^(u32 p)   const { return powi(p); }
        Complex operator^(float p) const { return pow(p); }
        Complex<Flt> operator^(const Complex& p) const { return pow(p); }
        Complex& operator^=(u32 p)   { return *this = powi(p); }
        Complex& operator^=(float p) { return *this = pow(p); }
        Complex<Flt>& operator^=(const Complex& p) requires std::is_same_v<Flt, T> { return *this = pow(p); }

        bool operator==(const Complex&) const = default;

        template <class U> operator Complex<U>() const { return { (U)re, (U)im }; }

        static Complex random_rot(RandomGenerator& rg);
    };

    template <class T> Complex<T> operator+(T x, const Complex<T>& z) { return  z + x; }
    template <class T> Complex<T> operator-(T x, const Complex<T>& z) { return -z + x; }
    template <class T> Complex<T> operator*(T x, const Complex<T>& z) { return  z * x; }
    template <class T> Complex<typename Complex<T>::Flt> operator/(T x, const Complex<T>& z) { return z.inv() * x; }
    template <class T> Complex<typename Complex<T>::Flt> operator^(T x, const Complex<T>& z) { return Complex<T> { x } ^ z; }

    template <class T>
    typename VectorN<2, T>::float_vec VectorN<2, T>::rotated_by(const Complex<T>& rotation) const {
        return rotation.rotate(*this);
    }

    template <class T>
    VectorN<2, T>& VectorN<2, T>::rotate_by(const Complex<T>& rotation) requires traits_float {
        return *this = rotation.rotate(*this);
    }

    using iComplex = Complex<int>;
    using fComplex = Complex<float>;
    using dComplex = Complex<double>;

    inline dComplex operator ""_i(long double f) { return dComplex { 0, f }; }
    inline iComplex operator ""_i(u64 i) { return iComplex { 0, i }; }
}

namespace Quasi::Text {
    template <class T>
    struct Formatter<Math::Complex<T>> {
        static usize FormatTo(StringWriter output, const Math::Complex<T>& z, Str) {
            return Text::FormatTo(output, "{} {} {}i", z.re, z.im < 0 ? '-' : '+', z.im);
        }
    };
}