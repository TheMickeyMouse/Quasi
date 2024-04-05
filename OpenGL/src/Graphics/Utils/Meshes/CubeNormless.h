#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"
#include "NumTypes.h"

namespace Graphics::MeshUtils {
    struct CubeNormlessCreator;

    template <> struct OptionsFor<CubeNormlessCreator> {};

    struct CubeNormlessCreator : MeshConstructor<CubeNormlessCreator> {
        using MData = VertexBuilder::MeshConstructData3D;

        CubeNormlessCreator(Options) {}

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
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
                mesh.GetVertices().emplace_back(f(MData { .Position = Maths::fvec3 { i }, .VertexIndex = (uint)i }));
            }

            return mesh;
        }
    };

    inline static MeshConstructor<CubeNormlessCreator> CubeNormless {};
}
