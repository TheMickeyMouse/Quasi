#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"
#include "NumTypes.h"

namespace Graphics::MeshUtils {
    struct CubeCreator;

    template <> struct OptionsFor<CubeCreator> {};

    struct CubeCreator : MeshConstructor<CubeCreator> {
        using MData = VertexBuilder::MeshConstructData3D;

        CubeCreator(Options) {}

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
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
                vert.emplace_back(f(MData { .Position = Maths::fvec3 { (Maths::Corner3D)(t1 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 0 }));
                vert.emplace_back(f(MData { .Position = Maths::fvec3 { (Maths::Corner3D)(t2 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 1 }));
                vert.emplace_back(f(MData { .Position = Maths::fvec3 { (Maths::Corner3D)(t3 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 2 }));
                vert.emplace_back(f(MData { .Position = Maths::fvec3 { (Maths::Corner3D)(t4 ^ flip) }, .Normal = norm, .VertexIndex = indoff + 3 }));
            }

            return mesh;
        }
    };

    inline static MeshConstructor<CubeCreator> Cube {};
}
