#pragma once

#include "Constants.h"

namespace Graphics::MeshUtils {
    template <stdu::fn_args<const VertexBuilder::MeshConstructData2D&> F>
    auto Circle(uint subdivisions, F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData2D {}))> {
        using Mdata = VertexBuilder::MeshConstructData2D;
        using T = decltype(f(Mdata {}));
        const float angle = Maths::TAU / (float)subdivisions;
        Mesh<T> mesh;
        mesh.GetVertices().reserve(subdivisions + 1);
        mesh.GetIndices().reserve(subdivisions);

        mesh.GetVertices().emplace_back(f(Mdata { Maths::fvec2::ZERO(), 0 }));
        for (uint i = 0; i < subdivisions; ++i) {
            mesh.GetVertices().push_back(f(Mdata { Maths::fvec2::from_polar(1.0f, angle * (float)i), i + 1 }));
            mesh.GetIndices().emplace_back(0, i + 1, (i + 1) % subdivisions + 1);
        }

        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto CubeNormless(F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        Mesh<T> mesh;
        mesh.GetIndices().reserve(6 * 2);
        for (Maths::Direction3D i = Maths::Direction3D::RIGHT; i < 6; ++i) {
            // some magic that calculates quad indices
            const uint ii = (uint)i, n = ii / 2, flip = (ii & 1) << n;
            const uint t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
            mesh.GetIndices().emplace_back(t1 ^ flip, t2 ^ flip, t3 ^ flip);
            mesh.GetIndices().emplace_back(t2 ^ flip, t3 ^ flip, t4 ^ flip);
        }

        mesh.GetVertices().reserve(8);
        for (Maths::Corner3D i = Maths::Corner3D::FRONT_TOP_RIGHT; i < 8; ++i) {
            mesh.GetVertices().emplace_back(f(Mdata { .Position = Maths::fvec3 { i }, .VertexIndex = (uint)i }));
        }

        return mesh;
    }

    template <stdu::fn_args<const VertexBuilder::MeshConstructData3D&> F>
    auto Cube(F&& f) -> Mesh<decltype(f(VertexBuilder::MeshConstructData3D {}))> {
        using Mdata = VertexBuilder::MeshConstructData3D;
        using T = decltype(f(Mdata {}));
        Mesh<T> mesh;
        auto& vert = mesh.GetVertices();
        auto& ind = mesh.GetIndices();
        vert.reserve(6 * 4);
        ind.reserve(6 * 2);
        for (Maths::Direction3D i = Maths::Direction3D::RIGHT; i < 6; ++i) {
            const uint ii = (uint)i, indoff = ii * 4;
            ind.emplace_back(indoff, indoff + 1, indoff + 2);
            ind.emplace_back(indoff + 1, indoff + 2, indoff + 3);
            const Maths::fvec3 norm { i };
            const uint n = ii / 2, flip = (ii & 1) << n;
            const uint t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t1 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 0 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t2 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 1 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t3 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 2 }));
            vert.emplace_back(f(Mdata { .Position = Maths::fvec3 { (Maths::Corner3D)(t4 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 3 }));
        }

        return mesh;
    }
}
