#pragma once

#include "Geometry.h"
#include "Mesh.h"
#include "Vector.h"

namespace Quasi::Graphics::Primitives {
    class Tri {
    private:
        using fVector3 = Math::fVector3;
        fVector3 a, b, c;
    public:
        Tri() = default;
        Tri(fVector3 a, fVector3 b, fVector3 c) : a(a), b(b), c(c) {}

        static Tri FromGeometry(Math::fTriangle3D geom);

        void Transform(const Math::Matrix3D& transform);
        friend Tri operator*(const Math::Matrix3D& transform, const Tri& mesh);

        template <class T> Mesh<T> IntoMesh(fVector3 T::* prop = &T::Position);
        template <class T, class F> Mesh<T> IntoMesh(F f, decltype(f(fVector3 {})) T::* prop = &T::Position);

        [[nodiscard]] fVector3 Origin() const { return (a + b + c) / 3.0f; }
        [[nodiscard]] fVector3 Normal() const { return (b - a).cross(c - a).norm(); }
        Tri& Center() { const fVector3 origin = Origin(); a -= origin; b -= origin; c -= origin; return *this; }

        template <class T> friend class Mesh;
    };

    Tri operator*(const Math::Matrix3D& transform, const Tri& mesh);

    template <class T>
    Mesh<T> Tri::IntoMesh(fVector3 T::* prop) {
        return IntoMesh(std::identity {}, prop);
    }

    template <class T, class F> Mesh<T> Tri::IntoMesh(F f, decltype(f(fVector3{})) T::* prop) {
        Vec<T> vert;
        vert.resize(3);
        vert[0].*prop = f(a);
        vert[1].*prop = f(b);
        vert[2].*prop = f(c);

        return Mesh<T> { std::move(vert), { { 0, 1, 2 } } };
    }
}

namespace Quasi::Graphics {
    template <class T>
    template <Fn<T, Math::fVector3> F> void Mesh<T>::AddTri(const Primitives::Tri& tri, F&& f) {
        const u32 i = (u32)vertices.size();
        vertices.emplace_back(f(tri.a));
        vertices.emplace_back(f(tri.b));
        vertices.emplace_back(f(tri.c));
        indices.emplace_back(i, i + 1, i + 2);
    }
}
