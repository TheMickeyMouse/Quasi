#pragma once

#include <array>

#include "Geometry.h"
#include "GraphicsDevice.h"
#include "Mesh.h"
#include "Vector.h"

namespace Graphics::Primitives {
    class Tri {
    private:
        using fvec3 = Maths::fvec3;
        fvec3 a, b, c;
    public:
        Tri() = default;
        Tri(fvec3 a, fvec3 b, fvec3 c) : a(a), b(b), c(c) {}

        OPENGL_API static Tri FromGeometry(Maths::Geometry::ftriangle3d geom);

        OPENGL_API std::array<fvec3, 3> GetVertices() const;

        OPENGL_API void Transform(const Maths::mat3D& transform);
        friend OPENGL_API Tri operator*(const Maths::mat3D& transform, const Tri& mesh);

        template <class T> Mesh<T> IntoMesh(fvec3 T::* prop = &T::Position);
        template <class T, class F> Mesh<T> IntoMesh(F f, decltype(f(fvec3 {})) T::* prop = &T::Position);
    };

    OPENGL_API Tri operator*(const Maths::mat3D& transform, const Tri& mesh);

    template <class T>
    Mesh<T> Tri::IntoMesh(fvec3 T::* prop) {
        return IntoMesh([](const fvec3& v) -> fvec3 { return v; }, prop);
    }

    template <class T, class F> Mesh<T> Tri::IntoMesh(F f, decltype(f(fvec3{})) T::* prop) {
        std::vector<T> vert;
        auto v = GetVertices();

        vert.resize(3);
        vert[0].*prop = f(v[0]);
        vert[1].*prop = f(v[1]);
        vert[2].*prop = f(v[2]);

        return Mesh<T> { std::move(vert), { { 0, 1, 2 } } };
    }
}
