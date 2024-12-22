#pragma once

#include "Rect.h"

namespace Quasi::Math {
#define NODISC [[nodiscard]]
    // column major mat
    namespace details {
        template <u32 D>
        using rotation_scalar = IfElse<(D == 2), float, IfElse<(D == 3), fVector3, Empty>>;
    }

    template <u32 N, u32 M> // N is the dimension of each vector, M is the number of vectors
    struct Matrix {
        using col = VectorN<N, float>;
        using row = VectorN<M, float>;
        using vec  = VectorN<N - 1, float>;
        using rvec = VectorN<M - 1, float>;
        static constexpr bool is_square = N == M;

        VectorN<M, col> mat { col::ZERO() };

    private:
        void construct_identity() requires is_square;
    public:
        Matrix() requires is_square { construct_identity(); }
        template <class... Vs>
        Matrix(const Vs&... cols) requires (ConvTo<Vs, col> && ...) : mat { cols... } {}
        Matrix(const VectorN<M, col>& mat) : mat(mat) {}

        static Matrix identity() requires is_square { return Matrix {}; }
        static Matrix from_span(Span<const float> data); // column major

        NODISC Span<const float> data() const;
        NODISC Span<const col> get_cols() const;

        NODISC vec translation() const { return (vec)mat.last(); }
        NODISC Matrix<N - 1, M - 1> linear_matrix() const;
        NODISC Matrix<N + 1, M + 1> affine_matrix() const;

        Matrix& translate(const vec& translation);
        Matrix& scale(const vec& scale);
        Matrix& scale_linear(const col& scale) requires is_square;
        Matrix& rotate(const details::rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4));
        Matrix& rotate_linear(const details::rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3));
        static Matrix translate_mat(const vec& translation) requires is_square;
        static Matrix scale_mat(const vec& scale) requires is_square;
        static Matrix scale_mat_linear(const col& scale) requires is_square;
        static Matrix rotate_mat(const details::rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4));
        static Matrix rotate_mat_linear(const details::rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3));

        static Matrix rotate_x_linear(float roll)  requires (is_square &&  N == 3);
        static Matrix rotate_y_linear(float pitch) requires (is_square &&  N == 3);
        static Matrix rotate_z_linear(float yaw)   requires (is_square && (N == 2 || N == 3));
        static Matrix rotate_x(float roll)  requires (is_square &&  N == 4) { return Matrix<N - 1, M - 1>::rotate_x_linear(roll).affine_matrix(); }
        static Matrix rotate_y(float pitch) requires (is_square &&  N == 4) { return Matrix<N - 1, M - 1>::rotate_y_linear(pitch).affine_matrix(); }
        static Matrix rotate_z(float yaw)   requires (is_square && (N == 3 || N == 4)) { return Matrix<N - 1, M - 1>::rotate_z_linear(yaw).affine_matrix(); }

        static Matrix ortho_projection(const RectN<N - 1, float>& box) requires is_square;
        static Matrix perspective_projection(const RectN<N - 1, float>& box) requires is_square;
        static Matrix perspective_fov(float fovDeg, float aspect, float near, float far) requires (is_square && N == 4);
        static Matrix transform(const vec& translate, const vec& scale, const details::rotation_scalar<N - 1>& rotate) requires (is_square && (N == 3 || N == 4));
        static Matrix look_at(const fVector3& eye, const fVector3& center, const fVector3& worldUp) requires (is_square && N == 4);

        NODISC Matrix<M, N> transpose() const;
        NODISC float det() const requires is_square; // determinate
        NODISC Matrix inv() const requires is_square; // inverse
        NODISC float trace() const requires is_square;

        template <u32 P> NODISC Matrix<P, M> then(const Matrix<P, N>& next) const;

        NODISC Matrix squared() const requires is_square;

        template <u32 Ig> NODISC Matrix<N - Ig, M - Ig> skip(Array<u8, Ig> skipped) const;

        NODISC const col& operator[](usize i) const { return mat[i]; }
        col& operator[](usize i) { return mat[i]; }

        NODISC Matrix  operator* (float s) const;
               Matrix& operator*=(float s);

        NODISC Matrix  operator+ (const Matrix& m) const;
               Matrix& operator+=(const Matrix& m);

        NODISC vec operator*(const rvec& v) const;
        NODISC col operator*(const row&  r) const;

        template <u32 P> NODISC Matrix<N, P> operator*(const Matrix<M, P>& m) const;

        // this APPLIES m to this. not the other way around. essentially: this = m * this;
        Matrix& operator*=(const Matrix<N, N>& m) { *this = m * *this; return *this; }
    };
#undef NODISC

    using Matrix2x2 = Matrix<2, 2>;
    using Matrix2x3 = Matrix<2, 3>;
    using Matrix2x4 = Matrix<2, 4>;
    using Matrix3x2 = Matrix<3, 2>;
    using Matrix3x3 = Matrix<3, 3>;
    using Matrix3x4 = Matrix<3, 4>;
    using Matrix4x2 = Matrix<4, 2>;
    using Matrix4x3 = Matrix<4, 3>;
    using Matrix4x4 = Matrix<4, 4>;

    using Matrix2D = Matrix3x3;
    using Matrix3D = Matrix4x4;
}