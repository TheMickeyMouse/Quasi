#include "Matrix.h"

#include "Constants.h"

namespace Quasi::Math {
    template <u32 N, u32 M> void Matrix<N, M>::construct_identity() requires is_square {
        for (u32 i = 0; i < N; ++i) {
            mat[i][i] = 1.0f;
        }
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::from_span(Span<const float> data) {
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return Matrix { col::from_span(data.subspan(N * I))... };
        }(std::make_integer_sequence<u32, M> {});
    }

    template <u32 N, u32 M> Span<const float, N * M> Matrix<N, M>::data() const {
        return std::span { mat.first().begin(), N * M }.template first<N * M>();
    }

    template <u32 N, u32 M> auto Matrix<N, M>::get_cols() const -> Span<const col, M> {
        return std::span { mat }.template first<M>();
    }

    template <u32 N, u32 M> Matrix<N - 1, M - 1> Matrix<N, M>::linear_matrix() const {
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return Matrix<N - 1, M - 1> { mat[I].shrink()... };
        }(std::make_integer_sequence<u32, M - 1> {});
    }

    template <u32 N, u32 M> Matrix<N + 1, M + 1> Matrix<N, M>::affine_matrix() const {
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            Matrix<N + 1, M + 1> m { mat[I].extend(0.0f)..., VectorN<N + 1, float>::ZERO() };
            m[M][N] = 1.0f;
            return m;
        }(std::make_integer_sequence<u32, M> {});
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::translate(const vec& translation) {
        mat.last() += translation.extend(0.0f);
        return *this;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::translate_mat(const vec& translation) requires is_square {
        Matrix identity {};
        identity.mat.last() = translation.extend(1.0f);
        return identity;
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::scale(const vec& scale) {
        for (u32 i = 0; i < M - 1; ++i)
            mat[i] *= scale.extend(1);
        return *this;
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::scale_linear(const col& scale) requires is_square {
        for (u32 i = 0; i < M; ++i)
            mat[i] *= scale;
        return *this;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::scale_mat(const vec& scale) requires is_square {
        Matrix identity {};
        for (u32 i = 0; i < N - 1; ++i) {
            identity.mat[i][i] = scale[i];
        }
        return identity;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::scale_mat_linear(const col& scale) requires is_square {
        Matrix identity {};
        for (u32 i = 0; i < N; ++i) {
            identity.mat[i][i] = scale[i];
        }
        return identity;
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::rotate(const details::rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4)) {
        return *this = rotate_mat(rotation) * *this;
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::rotate_linear(const details::rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3)) {
        return *this = rotate_mat_linear(rotation) * *this;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::rotate_mat(const details::rotation_scalar<N - 1>& rotation) requires (is_square && (N == 3 || N == 4)) {
        if      constexpr (N == 3) return Matrix::rotate_z(rotation);
        else if constexpr (N == 4) return Matrix::rotate_z(rotation.z).then(Matrix::rotate_x(rotation.x)).then(Matrix::rotate_y(rotation.y));
        else return nullptr;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::rotate_mat_linear(const details::rotation_scalar<N>& rotation) requires (is_square && (N == 2 || N == 3)) {
        if      constexpr (N == 2) return Matrix::rotate_z_linear(rotation);
        else if constexpr (N == 3) return Matrix::rotate_z_linear(rotation.z).then(Matrix::rotate_x_linear(rotation.x)).then(Matrix::rotate_y_linear(rotation.y));
        else return nullptr;
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::rotate_x_linear(float roll) requires (is_square && N == 3) {
        const float cos = std::cos(roll), sin = std::sin(roll);
        return { col { 1,   0,    0 },
                 col { 0, cos, -sin },
                 col { 0, sin,  cos } };
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::rotate_y_linear(float pitch) requires (is_square && N == 3) {
        const float cos = std::cos(pitch), sin = std::sin(pitch);
        return { col { cos, 0, -sin },
                 col {   0, 1,    0 },
                 col { sin, 0,  cos } };
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::rotate_z_linear(float yaw) requires (is_square && (N == 2 || N == 3)) {
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

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::ortho_projection(const RectN<N - 1, float>& box) requires is_square {
        // a bit of explaination: (dx is diff or `n_distance(I)`, sx is sum or `center()[I] * 2`)
        // [ 2 / dx ,   0    ,    0    , -sx / dx ]
        // [   0    , 2 / dy ,    0    , -sy / dy ]
        // [   0    ,   0    , -2 / dz , -sz / dz ]
        // [   0    ,   0    ,    0    ,     1    ]
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return Matrix {
                col::from_direction((2 * I), (I == 2 ? -2.0f : 2.0f) / box.n_distance(I))..., // cols 1~next to last created in a direction
                col { (-2 * box.center()[I] / box.n_distance(I))..., 1.0f } // last col with translation, and constant 1 w
            };
        }(std::make_integer_sequence<u32, N - 1> {});
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::perspective_projection(const RectN<N - 1, float>& box) requires is_square {
        // heres the math: http://www.songho.ca/opengl/gl_projectionMatrix.html
        // (dx is diff or `n_distance(I)`, sx is sum or `center()[I] * 2`), n is near, f is far
        // { 2 * n / dx ,     0      ,  sx / dx ,       0         }
        // {     0      , 2 * n / dy ,  sy / dy ,       0         }
        // {     0      ,     0      , -sz / dz , -2 * n * f / dz }
        // {     0      ,     0      ,    -1    ,       0         }
        const float near = box.min.last(), far = box.max.last();
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return Matrix {
                // first few cols
                col::from_direction((2 * I), 2 * near / box.n_distance(I))...,
                // z component, first few are 0
                col { (2 * box.center()[I] / box.n_distance(I))..., -2 * box.center()[N - 2] / box.n_distance(N - 2), -1 },
                // w component
                col { 0 * I..., -2 * near * far * box.n_distance(N - 2), 0 }
            };
        }(std::make_integer_sequence<u32, N - 2> {});
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::perspective_fov(float fovDeg, float aspect, float near, float far) requires (is_square && N == 4) {
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
        return Matrix {
            col { x, 0,      0,      0 },
            col { 0, y,      0,      0 },
            col { 0, 0,   -zsum,    -1 },
            col { 0, 0, -2 * zprod,  0 }
        };
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::transform(const vec& translate, const vec& scale, const details::rotation_scalar<N - 1>& rotate) requires (is_square && (N == 3 || N == 4)) {
        return scale_mat(scale).rotate(rotate).translate(translate);
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::look_at(const fVector3& eye, const fVector3& center, const fVector3& worldUp) requires (is_square && N == 4) {
        // from this answer: https://stackoverflow.com/a/21830596
        // z is front
        const fVector3 viewZ = (eye - center).norm();
        // getting the vector perpendicular to the plane (eye -> center) and worldUp
        const fVector3 viewX = worldUp.cross(viewZ).norm();
        // getting localUp from viewX and viewZ
        const fVector3 viewY = viewZ.cross(viewX);
        return {
            col { viewX.x,          viewY.x,         viewZ.x,        0 },
            col { viewX.y,          viewY.y,         viewZ.y,        0 },
            col { viewX.z,          viewY.z,         viewZ.z,        0 },
            col { -viewX.dot(eye), -viewY.dot(eye), -viewZ.dot(eye), 1 },
        };
    }

    template <u32 N, u32 M> Matrix<M, N> Matrix<N, M>::transpose() const {
        Matrix<M, N> t;
        for (u32 i = 0; i < M; ++i) {
            for (u32 j = 0; j < N; ++j) {
                t[j][i] = mat[i][j];
            }
        }
        return t;
    }

    template <u32 N, u32 M> float Matrix<N, M>::det() const requires is_square {
        if constexpr (N == 1) return mat[0][0];
        else if constexpr (N == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
        else if constexpr (N == 3) return mat[0].dot(mat[1].cross(mat[2]));
        else {
            // LU decomp
            // https://www.youtube.com/watch?v=BFYFkn-eOQk
            Matrix U = *this;
            for (u32 col = 0; col < M; ++col) {
                for (u32 row = col + 1; row < N; ++row) {
                    float factor = U[col][row] / U[col][col];

                    for (u32 i = col; i < M; ++i) U[i][row] -= U[i][col] * factor;
                }
            }

            float deter = U[0][0];
            for (u32 i = 1; i < N; ++i) deter *= U[i][i];
            return deter;
        }
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::inv() const requires is_square {
        // a bit cheaty but fast
        // https://en.wikipedia.org/wiki/Invertible_matrix#Analytic_solution
        float deter = det();
        const Matrix& self = *this;
        if constexpr (N == 2) {
            return Matrix {
                col { +self[1][1] / deter, -self[1][0] / deter },
                col { -self[0][1] / deter, +self[0][0] / deter },
            };
        } else if constexpr (N == 3) {
            return Matrix {
                self[1].cross(self[2]),
                self[2].cross(self[0]),
                self[0].cross(self[1])
            }.transpose() * (1 / deter);
        } else if constexpr (N == 4) {
            const Matrix A2 = squared(), A3 = A2 * self;
            const float trA = trace(), trA2 = A2.trace(), trA3 = A3.trace();

            float factors[4] = {
                ((trA * trA * trA) - 3 * trA * trA2 + 2 * trA3) / +6.0f / deter,
                ((trA * trA)       -           trA2           ) / -2.0f / deter,
                trA                                                     / deter,
                -1.0f                                                   / deter
            };

            return Matrix::identity() * factors[0] + self * factors[1] + A2 * factors[2] + A3 * factors[3];
        } else {
            return "haven't implemented this high of a dimension. bruh";
        }
    }

    template <u32 N, u32 M> float Matrix<N, M>::trace() const requires is_square {
        float tr = (*this)[0][0];
        for (u32 i = 1; i < N; ++i) tr += (*this)[i][i];
        return tr;
    }

    template <u32 N, u32 M> template <u32 P> Matrix<P, M> Matrix<N, M>::then(const Matrix<P, N>& next) const {
        return next * (*this);
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::squared() const requires is_square {
        const Matrix& self = *this;
        return self * self;
    }

    template <u32 N, u32 M> template <u32 Ig>
    Matrix<N - Ig, M - Ig> Matrix<N, M>::skip(Array<u8, Ig> skipped) const {
        Matrix<N - Ig, M - Ig> matrix;
        u32 icol = 0, irow = 0;
        u32 colMask = 0, rowMask = 0;

        for (const u8 colRowPair : skipped) { // cache skipped into bitmask
            colMask |= 1 << (colRowPair >> 4);
            rowMask |= 1 << (colRowPair & 15);
        }

        for (u32 col = 0; col < M; ++col) {
            if (colMask & (1 << col)) continue;
            for (u32 row = 0; row < N; ++row) {
                if (rowMask & (1 << row)) continue;
                matrix[icol][irow] = (*this)[col][row];
                ++irow;
            }
            ++icol;
        }
        return matrix;
    }

    template <u32 N, u32 M>
    Matrix<N, M> Matrix<N, M>::operator*(float s) const {
        return mat * s;
    }

    template <u32 N, u32 M>
    Matrix<N, M>& Matrix<N, M>::operator*=(float s) {
        mat *= s; return *this;
    }

    template <u32 N, u32 M> Matrix<N, M> Matrix<N, M>::operator+(const Matrix& m) const {
        return [&]<u32... I>(std::integer_sequence<u32, I...>) {
            return Matrix { (mat[I] + m[I])... };
        } (std::make_integer_sequence<u32, M> {});
    }

    template <u32 N, u32 M> Matrix<N, M>& Matrix<N, M>::operator+=(const Matrix& m) {
        [&]<u32... I>(std::integer_sequence<u32, I...>) {
            Empty _ { (mat[I] += m[I])... };
        } (std::make_integer_sequence<u32, M> {});
        return *this;
    }

    template <u32 N, u32 M>
    typename Matrix<N, M>::vec Matrix<N, M>::operator*(const rvec& v) const {
        return (*this) * v.extend(1.0f);
    }

    template <u32 N, u32 M>
    typename Matrix<N, M>::col Matrix<N, M>::operator*(const row& r) const {
        col res = col::ZERO();
        for (u32 i = 0; i < M; ++i) {
            res += mat[i] * r[i];
        }
        return res;
    }

    template <u32 N, u32 M>
    template <u32 P>
    Matrix<N, P> Matrix<N, M>::operator*(const Matrix<M, P>& m) const {
        Matrix<N, P> res;
        for (u32 i = 0; i < P; ++i) {
            res[i] = (*this) * m[i];
        }
        return res;
    }

    template struct Matrix<2, 2>;
    template struct Matrix<2, 3>;
    template struct Matrix<2, 4>;
    template struct Matrix<3, 2>;
    template struct Matrix<3, 3>;
    template struct Matrix<3, 4>;
    template struct Matrix<4, 2>;
    template struct Matrix<4, 3>;
    template struct Matrix<4, 4>;

    template Matrix<2, 2> Matrix<2, 2>::then(const Matrix<2, 2>&) const;
    template Matrix<3, 2> Matrix<2, 2>::then(const Matrix<3, 2>&) const;
    template Matrix<4, 2> Matrix<2, 2>::then(const Matrix<4, 2>&) const;
    template Matrix<2, 3> Matrix<2, 3>::then(const Matrix<2, 2>&) const;
    template Matrix<3, 3> Matrix<2, 3>::then(const Matrix<3, 2>&) const;
    template Matrix<4, 3> Matrix<2, 3>::then(const Matrix<4, 2>&) const;
    template Matrix<2, 4> Matrix<2, 4>::then(const Matrix<2, 2>&) const;
    template Matrix<3, 4> Matrix<2, 4>::then(const Matrix<3, 2>&) const;
    template Matrix<4, 4> Matrix<2, 4>::then(const Matrix<4, 2>&) const;
    template Matrix<2, 2> Matrix<3, 2>::then(const Matrix<2, 3>&) const;
    template Matrix<3, 2> Matrix<3, 2>::then(const Matrix<3, 3>&) const;
    template Matrix<4, 2> Matrix<3, 2>::then(const Matrix<4, 3>&) const;
    template Matrix<2, 3> Matrix<3, 3>::then(const Matrix<2, 3>&) const;
    template Matrix<3, 3> Matrix<3, 3>::then(const Matrix<3, 3>&) const;
    template Matrix<4, 3> Matrix<3, 3>::then(const Matrix<4, 3>&) const;
    template Matrix<2, 4> Matrix<3, 4>::then(const Matrix<2, 3>&) const;
    template Matrix<3, 4> Matrix<3, 4>::then(const Matrix<3, 3>&) const;
    template Matrix<4, 4> Matrix<3, 4>::then(const Matrix<4, 3>&) const;
    template Matrix<2, 2> Matrix<4, 2>::then(const Matrix<2, 4>&) const;
    template Matrix<3, 2> Matrix<4, 2>::then(const Matrix<3, 4>&) const;
    template Matrix<4, 2> Matrix<4, 2>::then(const Matrix<4, 4>&) const;
    template Matrix<2, 3> Matrix<4, 3>::then(const Matrix<2, 4>&) const;
    template Matrix<3, 3> Matrix<4, 3>::then(const Matrix<3, 4>&) const;
    template Matrix<4, 3> Matrix<4, 3>::then(const Matrix<4, 4>&) const;
    template Matrix<2, 4> Matrix<4, 4>::then(const Matrix<2, 4>&) const;
    template Matrix<3, 4> Matrix<4, 4>::then(const Matrix<3, 4>&) const;
    template Matrix<4, 4> Matrix<4, 4>::then(const Matrix<4, 4>&) const;
}
