#pragma once

#include <array>

#include "GraphicsDevice.h"
#include "Mesh.h"
#include "Vector.h"

namespace Graphics::Primitives {
    class Quad {
        private:
            using fvec3 = Maths::fvec3;
            fvec3 center, x, y;
        public:
            Quad() = default;
            Quad(fvec3 center, fvec3 x, fvec3 y) : center(center), x(x), y(y) {}
            
            OPENGL_API static Quad FromCornerToCorners(fvec3 corner, fvec3 toC1, fvec3 toC2);
            OPENGL_API static Quad FromCenterToCorners(fvec3 center, fvec3 toC1, fvec3 toC2);
                
            OPENGL_API std::array<fvec3, 4> GetVertices() const;

            OPENGL_API void Transform(const Maths::mat3D& transform);
            friend OPENGL_API Quad operator*(const Maths::mat3D& transform, const Quad& mesh);

            template <class T> Mesh<T> IntoMesh(fvec3 T::* prop = &T::Position);
            template <class T, class F> Mesh<T> IntoMesh(F f, decltype(f(fvec3 {})) T::* prop = &T::Position);
    };

    OPENGL_API Quad operator*(const Maths::mat3D& transform, const Quad& mesh);

    template <class T>
    Mesh<T> Quad::IntoMesh(fvec3 T::* prop) {
        return IntoMesh([](const fvec3& v) -> fvec3 { return v; }, prop);
    }

    template <class T, class F> Mesh<T> Quad::IntoMesh(F f, decltype(f(fvec3{})) T::* prop) {
        std::vector<T> vert;
        auto v = GetVertices();

        vert.resize(4);
        vert[0].*prop = f(v[0]);
        vert[1].*prop = f(v[1]);
        vert[2].*prop = f(v[2]);
        vert[3].*prop = f(v[3]);

        return Mesh<T> { std::move(vert), { { 0, 1, 2 }, { 1, 2, 3 } } };
    }
}
