#include "Matrix.h"

#include "Constants.h"

namespace Maths {
    template <uint N, uint M> void matrix<N, M>::construct_identity() requires is_square {
        for (uint i = 0; i < N; ++i) {
            mat[i][i] = 1.0f;
        }
    }

    template <uint N, uint M> std::span<const float, N * M> matrix<N, M>::data() const {
        return std::span { mat.first().begin(), N * M }.template first<N * M>();
    }

    template <uint N, uint M> auto matrix<N, M>::get_cols() const -> std::span<const col, M> {
        return std::span { mat }.template first<M>();
    }

    template <uint N, uint M> matrix<N - 1, M - 1> matrix<N, M>::linear_matrix() const {
        return [&]<uint... I>(std::integer_sequence<uint, I...>) {
            return matrix<N - 1, M - 1> { mat[I].shrink()... };
        }(std::make_integer_sequence<uint, M - 1> {});
    }

    template <uint N, uint M> matrix<N + 1, M + 1> matrix<N, M>::affine_matrix() const {
        return [&]<uint... I>(std::integer_sequence<uint, I...>) {
            matrix<N + 1, M + 1> m { mat[I].extend(0.0f)..., vecn<N + 1, float>::ZERO() };
            m[M][N] = 1.0f;
            return m;
        }(std::make_integer_sequence<uint, M> {});
    }

    template <uint N, uint M> matrix<N, M>& matrix<N, M>::translate(const vec& translation) {
        mat.last() += translation.extend(0.0f);
        return *this;
    }

    template <uint N, uint M> matrix<N, M> matrix<N, M>::translate_mat(const vec& translation) requires is_square {
        matrix identity {};
        identity.mat.last() = translation.extend(1.0f);
        return identity;
    }

    template <uint N, uint M> matrix<N, M>& matrix<N, M>::scale(const vec& scale) {
        for (uint i = 0; i < M - 1; ++i)
            mat[i] *= scale.extend(1);
        return *this;
    }

    template <uint N, uint M> matrix<N, M>& matrix<N, M>::scale_linear(const col& scale) requires is_square {
        for (uint i = 0; i < M; ++i)
            mat[i] *= scale;
        return *this;
    }

    template <uint N, uint M> matrix<N, M> matrix<N, M>::scale_mat(const vec& scale) requires is_square {
        matrix identity {};
        for (uint i = 0; i < N - 1; ++i) {
            identity.mat[i][i] = scale[i];
        }
        return identity;
    }

    template <uint N, uint M> matrix<N, M> matrix<N, M>::scale_mat_linear(const col& scale) requires is_square {
        matrix identity {};
        for (uint i = 0; i < N; ++i) {
            identity.mat[i][i] = scale[i];
        }
        return identity;
    }

    template <uint N, uint M> matrix<N, M>& matrix<N, M>::rotate(const rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4)) {
        return *this = rotate_mat(rotation) * *this;
    }

    template <uint N, uint M> matrix<N, M>& matrix<N, M>::rotate_linear(const rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3)) {
        return *this = rotate_mat_linear(rotation) * *this;
    }

    template <uint N, uint M> matrix<N, M> matrix<N, M>::rotate_mat(const rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4)) {
        if      constexpr (N == 3) return matrix::rotate_z(rotation);
        else if constexpr (N == 4) return matrix::rotate_y(rotation.y) * matrix::rotate_x(rotation.x) * matrix::rotate_z(rotation.z);
        else return nullptr;
    }

    template <uint N, uint M> matrix<N, M> matrix<N, M>::rotate_mat_linear(const rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3)) {
        if      constexpr (N == 2) return matrix::rotate_z_linear(rotation);
        else if constexpr (N == 3) return matrix::rotate_y_linear(rotation.y) * matrix::rotate_x_linear(rotation.x) * matrix::rotate_z(rotation.z);
        else return nullptr;
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::rotate_x_linear(float roll) requires (is_square && N == 3) {
        const float cos = std::cos(roll), sin = std::sin(roll);
        return { col { 1,   0,    0 },
                 col { 0, cos, -sin },
                 col { 0, sin,  cos } };
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::rotate_y_linear(float pitch) requires (is_square && N == 3) {
        const float cos = std::cos(pitch), sin = std::sin(pitch);
        return { col { cos, 0, -sin },
                 col {   0, 1,    0 },
                 col { sin, 0,  cos } };
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::rotate_z_linear(float yaw) requires (is_square && (N == 2 || N == 3)) {
        if constexpr (N == 3) {
            const float cos = std::cos(yaw), sin = std::sin(yaw);
            return { col {  cos, sin, 0 },
                     col { -sin, cos, 0 },
                     col {    0,   0, 1 } };
        } else {
            const float cos = std::cos(yaw), sin = std::sin(yaw);
            return { col {  cos, sin },
                     col { -sin, cos } };
        }
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::ortho_projection(const rect<N - 1, float>& box) requires is_square {
        // a bit of explaination: (dx is diff or `n_distance(I)`, sx is sum or `center()[I] * 2`)
        // [ 2 / dx ,   0    ,    0    , -sx / dx ]
        // [   0    , 2 / dy ,    0    , -sy / dy ]
        // [   0    ,   0    , -2 / dz , -sz / dz ]
        // [   0    ,   0    ,    0    ,     1    ]
        return [&]<uint... I>(std::integer_sequence<uint, I...>) {
            return matrix {
                col { (DirectionND<N>)(2 * I), (I == 2 ? -2.0f : 2.0f) / box.n_distance(I) }..., // cols 1~next to last created in a direction
                col { (-2 * box.center()[I] / box.n_distance(I))..., 1.0f } // last col with translation, and constant 1 w
            };
        }(std::make_integer_sequence<uint, N - 1> {});
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::perspective_projection(const rect<N - 1, float>& box) requires is_square {
        // heres the math: http://www.songho.ca/opengl/gl_projectionmatrix.html
        // (dx is diff or `n_distance(I)`, sx is sum or `center()[I] * 2`), n is near, f is far
        // { 2 * n / dx ,     0      ,  sx / dx ,       0         }
        // {     0      , 2 * n / dy ,  sy / dy ,       0         }
        // {     0      ,     0      , -sz / dz , -2 * n * f / dz }
        // {     0      ,     0      ,    -1    ,       0         }
        const float near = box.min.last(), far = box.max.last();
        using Dir = DirectionND<N>;
        return [&]<uint... I>(std::integer_sequence<uint, I...>) {
            return matrix {
                // first few cols
                (col { (Dir)(2 * I), 2 * near / box.n_distance(I) })...,
                // z component, first few are 0
                col { (2 * box.center()[I] / box.n_distance(I))..., -2 * box.center()[N - 2] / box.n_distance(N - 2), -1 },
                // w component
                col { 0 * I..., -2 * near * far * box.n_distance(N - 2), 0 }
            };
        }(std::make_integer_sequence<uint, N - 2> {});
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::perspective_fov(float fovDeg, float aspect, float near, float far) requires (is_square && N == 4) {
        // y = 1 / tan(theta / 2),
        // x = y / aspect,
        // dz = far - near,
        // zsum  = (far + near) / dz,
        // zprod = (far * near) / dz,
        // [ x , 0 ,   0   ,      0     ]
        // [ 0 , y ,   0   ,      0     ]
        // [ 0 , 0 , -zsum , -2 * zprod ]
        // [ 0 , 0 ,  -1   ,      0     ]
        const float y = 1.0f / std::tan(fovDeg * DEG2RAD / 2),
                    x = y / aspect,
                    dz = far - near,
                    zsum  = (far + near) / dz,
                    zprod = (far * near) / dz;
        return matrix {
            col { x, 0,      0,      0 },
            col { 0, y,      0,      0 },
            col { 0, 0,   -zsum,    -1 },
            col { 0, 0, -2 * zprod,  0 }
        };
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::transform(const vec& translate, const vec& scale, const rotation_scalar<N - 1>& rotate) requires (is_square && (N == 3 || N == 4)) {
        return scale_mat(scale).rotate(rotate).translate(translate);
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::look_at(const fvec3& eye, const fvec3& center, const fvec3& worldUp) requires (is_square && N == 4) {
        // from this answer: https://stackoverflow.com/a/21830596
        // z is front
        const fvec3 viewZ = (eye - center).norm();
        // getting the vector perpendicular to the plane (eye -> center) and worldUp
        const fvec3 viewX = worldUp.cross(viewZ).norm();
        // getting localUp from viewX and viewZ
        const fvec3 viewY = viewZ.cross(viewX);
        return {
            col { viewX.x,          viewY.x,         viewZ.x,        0 },
            col { viewX.y,          viewY.y,         viewZ.y,        0 },
            col { viewX.z,          viewY.z,         viewZ.z,        0 },
            col { -viewX.dot(eye), -viewY.dot(eye), -viewZ.dot(eye), 1 },
        };
    }

    template <uint N, uint M> matrix<M, N> matrix<N, M>::transpose() const {
        matrix<M, N> t;
        for (uint i = 0; i < M; ++i) {
            for (uint j = 0; j < N; ++j) {
                t[j][i] = mat[i][j];
            }
        }
        return t;
    }

    template <uint N, uint M> float matrix<N, M>::det() const requires is_square {
        if constexpr (N == 1) return mat[0][0];
        if constexpr (N == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
        else return subdet<>();
    }

    template <uint N, uint M>
    template <matrix_entry... Ig>
    float matrix<N, M>::subdet() const requires is_square {
        constexpr uint Rest = N - sizeof...(Ig);
        if constexpr (Rest == 1) {
            constexpr uint Xs = N * (N - 1) / 2 - (Ig.I + ...),
                           Ys = N * (N - 1) / 2 - (Ig.J + ...);
            return mat[Xs][Ys];
        } else {
            float determ = 0.0f;
            bool alt = false;

            // main calculation, checks if cell is ignored, then recursively gets determinant
            const auto acc = [&]<matrix_entry E>() {
                if ((... || (E.I == Ig.I)) || (... || (E.J == Ig.J))) return;

                determ += (alt ? -1.0f : 1.0f) * mat[E.I][E.J] * this->subdet<E, Ig...>();
                alt = !alt;
            };

            // check if row is used, if it isn't go through all the rows and do calculation
            const auto find = [&]<uint J>() {
                if ((... || (J == Ig.J))) return false;

                [&]<uint... Is>(std::integer_sequence<uint, Is...>) {
                    stdu::empty _ = { (acc.template operator()<matrix_entry { Is, J }>(), 1)... };
                }(std::make_integer_sequence<uint, N> {});
                return true;
            };

            // find a row that hasn't been occupied (and then short-circuit with `||`)
            [&]<uint... Js>(std::integer_sequence<uint, Js...>) {
                (... || find.template operator()<Js>());
            }(std::make_integer_sequence<uint, N> {});

            return determ;
        }
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::adjugate() const requires is_square {
        matrix adj;
        [&]<matrix_entry... En>(stdu::valuelist<matrix_entry, En...>) {
            stdu::empty _ = { adj[En.I][En.J] = ((En.I + En.J & 1) ? -1.0f : 1.0f) * subdet<matrix_entry { En.J, En.I }>()... };
        }(make_matrix_entry_sequence<N, M> {});
        return adj;
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::inv() const requires is_square {
        return adjugate() * (1 / det());
    }

    template <uint N, uint M>
    matrix<N, M> matrix<N, M>::operator*(float s) const {
        return mat * s;
    }

    template <uint N, uint M>
    matrix<N, M>& matrix<N, M>::operator*=(float s) {
        mat *= s; return *this;
    }

    template <uint N, uint M>
    typename matrix<N, M>::vec matrix<N, M>::operator*(const rvec& v) const {
        return (*this) * v.extend(1.0f);
    }

    template <uint N, uint M>
    typename matrix<N, M>::col matrix<N, M>::operator*(const row& r) const {
        col res = col::ZERO();
        for (uint i = 0; i < M; ++i) {
            res += mat[i] * r[i];
        }
        return res;
    }

    template <uint N, uint M>
    template <uint P>
    matrix<N, P> matrix<N, M>::operator*(const matrix<M, P>& m) const {
        matrix<N, P> res;
        for (uint i = 0; i < P; ++i) {
            res[i] = (*this) * m[i];
        }
        return res;
    }

    template struct matrix<2, 2>;
    template struct matrix<2, 3>;
    template struct matrix<2, 4>;
    template struct matrix<3, 2>;
    template struct matrix<3, 3>;
    template struct matrix<3, 4>;
    template struct matrix<4, 2>;
    template struct matrix<4, 3>;
    template struct matrix<4, 4>;

    template OPENGL_API matrix<2, 2> matrix<2, 2>::operator*(const matrix<2, 2>&) const;
    template OPENGL_API matrix<2, 3> matrix<2, 2>::operator*(const matrix<2, 3>&) const;
    template OPENGL_API matrix<2, 4> matrix<2, 2>::operator*(const matrix<2, 4>&) const;
    template OPENGL_API matrix<2, 2> matrix<2, 3>::operator*(const matrix<3, 2>&) const;
    template OPENGL_API matrix<2, 3> matrix<2, 3>::operator*(const matrix<3, 3>&) const;
    template OPENGL_API matrix<2, 4> matrix<2, 3>::operator*(const matrix<3, 4>&) const;
    template OPENGL_API matrix<2, 2> matrix<2, 4>::operator*(const matrix<4, 2>&) const;
    template OPENGL_API matrix<2, 3> matrix<2, 4>::operator*(const matrix<4, 3>&) const;
    template OPENGL_API matrix<2, 4> matrix<2, 4>::operator*(const matrix<4, 4>&) const;
    template OPENGL_API matrix<3, 2> matrix<3, 2>::operator*(const matrix<2, 2>&) const;
    template OPENGL_API matrix<3, 3> matrix<3, 2>::operator*(const matrix<2, 3>&) const;
    template OPENGL_API matrix<3, 4> matrix<3, 2>::operator*(const matrix<2, 4>&) const;
    template OPENGL_API matrix<3, 2> matrix<3, 3>::operator*(const matrix<3, 2>&) const;
    template OPENGL_API matrix<3, 3> matrix<3, 3>::operator*(const matrix<3, 3>&) const;
    template OPENGL_API matrix<3, 4> matrix<3, 3>::operator*(const matrix<3, 4>&) const;
    template OPENGL_API matrix<3, 2> matrix<3, 4>::operator*(const matrix<4, 2>&) const;
    template OPENGL_API matrix<3, 3> matrix<3, 4>::operator*(const matrix<4, 3>&) const;
    template OPENGL_API matrix<3, 4> matrix<3, 4>::operator*(const matrix<4, 4>&) const;
    template OPENGL_API matrix<4, 2> matrix<4, 2>::operator*(const matrix<2, 2>&) const;
    template OPENGL_API matrix<4, 3> matrix<4, 2>::operator*(const matrix<2, 3>&) const;
    template OPENGL_API matrix<4, 4> matrix<4, 2>::operator*(const matrix<2, 4>&) const;
    template OPENGL_API matrix<4, 2> matrix<4, 3>::operator*(const matrix<3, 2>&) const;
    template OPENGL_API matrix<4, 3> matrix<4, 3>::operator*(const matrix<3, 3>&) const;
    template OPENGL_API matrix<4, 4> matrix<4, 3>::operator*(const matrix<3, 4>&) const;
    template OPENGL_API matrix<4, 2> matrix<4, 4>::operator*(const matrix<4, 2>&) const;
    template OPENGL_API matrix<4, 3> matrix<4, 4>::operator*(const matrix<4, 3>&) const;
    template OPENGL_API matrix<4, 4> matrix<4, 4>::operator*(const matrix<4, 4>&) const;
}
