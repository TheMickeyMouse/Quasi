#pragma once

#include <array>

#include "Mesh.h"
#include "Vector.h"

namespace Graphics::Primitives {
    class Quad {
    private:
        using fvec3 = Maths::fvec3;
        fvec3 center, x, y;
    public:
        Quad() = default;
        Quad(const fvec3& center, const fvec3& x, const fvec3& y) : center(center), x(x), y(y) {}

        static Quad FromCornerToCorners(const fvec3& corner, const fvec3& toC1, const fvec3& toC2);
        static Quad FromCenterToCorners(const fvec3& center, const fvec3& toC1, const fvec3& toC2);

        [[nodiscard]] std::array<fvec3, 4> GetVertices() const;

        void Transform(const Maths::mat3D& transform);
        friend Quad operator*(const Maths::mat3D& transform, const Quad& mesh);

        template <class T> Mesh<T> IntoMesh(fvec3 T::* prop = &T::Position);
        template <class T, class F>
        Mesh<T> IntoMesh(F f, std::remove_reference_t<decltype(f(fvec3 {}))> T::* prop = &T::Position);

        [[nodiscard]] fvec3 Origin() const { return center; }
        [[nodiscard]] fvec3 Normal() const { return x.cross(y).norm(); }
        Quad& Center() { center = 0; return *this; }
    };

    Quad operator*(const Maths::mat3D& transform, const Quad& mesh);

    template <class T>
    Mesh<T> Quad::IntoMesh(fvec3 T::* prop) {
        return IntoMesh(std::identity {}, prop);
    }

    template <class T, class F>
    Mesh<T> Quad::IntoMesh(F f, std::remove_reference_t<decltype(f(fvec3 {}))> T::* prop) {
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

namespace Graphics {
    template <class T>
    template <stdu::fn<T, Maths::fvec3> F>
    void Mesh<T>::AddQuad(const Primitives::Quad& quad, F&& f) {
        std::array v = quad.GetVertices();
        const uint i = (uint)vertices.size();
        vertices.emplace_back(f(v[0]));
        vertices.emplace_back(f(v[1]));
        vertices.emplace_back(f(v[2]));
        vertices.emplace_back(f(v[3]));
        indices.emplace_back(i, i + 1, i + 2);
        indices.emplace_back(i + 1, i + 2, i + 3);
    }
}