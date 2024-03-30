#pragma once
#include <core.h>
#include "Rect.h"

namespace Maths {
#define NODISC [[nodiscard]]
    // column major mat
    struct matrix_entry {
        uint I, J;
    };

    namespace detail {
        template <uint N, uint M, matrix_entry... Es> auto make_matrix_entry_sequence_impl() {
            if constexpr (M == 0) return stdu::valuelist<matrix_entry, Es...> {};
            else {
                return []<uint... Is>(std::integer_sequence<uint, Is...>) {
                    return make_matrix_entry_sequence_impl<N, M - 1, matrix_entry { M - 1, Is }..., Es...>();
                }(std::make_integer_sequence<uint, N> {});
            }
        }
    }

    template <uint N, uint M> using make_matrix_entry_sequence
        = decltype(detail::make_matrix_entry_sequence_impl<N, M>());

    template <uint N, uint M> // N is the dimension of each vector, M is the number of vectors
    struct matrix {
        using col = vecn<N, float>;
        using row = vecn<M, float>;
        using vec  = vecn<N - 1, float>;
        using rvec = vecn<M - 1, float>;
        static constexpr bool is_square = N == M;

        template <uint D>
        using rotation_scalar = std::conditional_t<D == 2, float, std::conditional_t<D == 3, fvec3, stdu::empty>>;

        vecn<M, col> mat { col::ZERO() };

    private:
        OPENGL_API void construct_identity() requires is_square;
    public:
        matrix() requires is_square { construct_identity(); }
        template <class... Vs>
        matrix(const Vs&... cols) requires (std::is_convertible_v<Vs, col> && ...) : mat { cols... } {}
        matrix(const vecn<M, col>& mat) : mat(mat) {}

        NODISC OPENGL_API std::span<const float, N * M> data() const;
        NODISC OPENGL_API std::span<const col, M> get_cols() const;

        NODISC OPENGL_API vec translation() const { return (vec)mat.last(); }
        NODISC OPENGL_API matrix<N - 1, M - 1> linear_matrix() const;
        NODISC OPENGL_API matrix<N + 1, M + 1> affine_matrix() const;

        OPENGL_API matrix& translate(const vec& translation);
        OPENGL_API matrix& scale(const vec& scale);
        OPENGL_API matrix& scale_linear(const col& scale) requires is_square;
        OPENGL_API matrix& rotate(const rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4));
        OPENGL_API matrix& rotate_linear(const rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3));
        OPENGL_API static matrix translate_mat(const vec& translation) requires is_square;
        OPENGL_API static matrix scale_mat(const vec& scale) requires is_square;
        OPENGL_API static matrix scale_mat_linear(const col& scale) requires is_square;
        OPENGL_API static matrix rotate_mat(const rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4));
        OPENGL_API static matrix rotate_mat_linear(const rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3));

        OPENGL_API static matrix rotate_x_linear(float roll)  requires (is_square &&  N == 3);
        OPENGL_API static matrix rotate_y_linear(float pitch) requires (is_square &&  N == 3);
        OPENGL_API static matrix rotate_z_linear(float yaw)   requires (is_square && (N == 2 || N == 3));
        OPENGL_API static matrix rotate_x(float roll)  requires (is_square &&  N == 4) { return matrix<N - 1, M - 1>::rotate_x_linear(roll).affine_matrix(); }
        OPENGL_API static matrix rotate_y(float pitch) requires (is_square &&  N == 4) { return matrix<N - 1, M - 1>::rotate_y_linear(pitch).affine_matrix(); }
        OPENGL_API static matrix rotate_z(float yaw)   requires (is_square && (N == 3 || N == 4)) { return matrix<N - 1, M - 1>::rotate_z_linear(yaw).affine_matrix(); }

        OPENGL_API static matrix ortho_projection(const rect<N - 1, float>& box) requires is_square;
        OPENGL_API static matrix perspective_projection(const rect<N - 1, float>& box) requires is_square;
        OPENGL_API static matrix perspective_fov(float fovDeg, float aspect, float near, float far) requires (is_square && N == 4);
        OPENGL_API static matrix transform(const vec& translate, const vec& scale, const rotation_scalar<N - 1>& rotate) requires (is_square && (N == 3 || N == 4));
        OPENGL_API static matrix look_at(const fvec3& eye, const fvec3& center, const fvec3& worldUp) requires (is_square && N == 4);

        NODISC OPENGL_API matrix<M, N> transpose() const;
        NODISC OPENGL_API float det() const requires is_square; // determinate
        template <matrix_entry... Ig> NODISC OPENGL_API float subdet() const requires is_square; // determinate
        NODISC OPENGL_API matrix adjugate() const requires is_square;
        NODISC OPENGL_API matrix inv() const requires is_square; // inverse

        NODISC OPENGL_API const col& operator[](uint i) const { return mat[i]; }
        NODISC OPENGL_API col& operator[](uint i) { return mat[i]; }

        NODISC OPENGL_API matrix  operator* (float s) const;
        OPENGL_API        matrix& operator*=(float s);

        NODISC OPENGL_API vec operator*(const rvec& v) const;
        NODISC OPENGL_API col operator*(const row&  r) const;

        template <uint P> NODISC OPENGL_API matrix<N, P> operator*(const matrix<M, P>& m) const;

        // this APPLIES m to this. not the other way around. essentially: this = m * this;
        matrix& operator*=(const matrix<N, N>& m) { *this = m * *this; return *this; }
    };

    using mat2x2 = matrix<2, 2>;
    using mat2x3 = matrix<2, 3>;
    using mat2x4 = matrix<2, 4>;
    using mat3x2 = matrix<3, 2>;
    using mat3x3 = matrix<3, 3>;
    using mat3x4 = matrix<3, 4>;
    using mat4x2 = matrix<4, 2>;
    using mat4x3 = matrix<4, 3>;
    using mat4x4 = matrix<4, 4>;

    using mat2D = mat3x3;
    using mat3D = mat4x4;
}