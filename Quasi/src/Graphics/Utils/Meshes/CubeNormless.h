#pragma once

#include "MeshConstructor.h"
#include "Graphicals/Mesh.h"

namespace Quasi::Graphics::VertexBuilder {
    struct MeshConstructData3D;
}

namespace Quasi::Graphics::MeshUtils {
    struct CubeNormlessCreator;

    template <> struct OptionsFor<CubeNormlessCreator> {
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct CubeNormlessCreator : MeshConstructor<CubeNormlessCreator> {
        CubeNormlessCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            auto meshp = mesh.NewBatch();
            for (u32 i = 0; i < 6; ++i) {
                // some magic that calculates quad indices
                const u32 n = i / 2, flip = (i & 1) << n;
                const u32 t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
                meshp.PushI(t1 ^ flip, t2 ^ flip, t3 ^ flip);
                meshp.PushI(t2 ^ flip, t3 ^ flip, t4 ^ flip);
            }

            for (u32 i = 0; i < 8; ++i) {
                meshp.PushV(f(MData { .Position = Math::fVector3::from_corner(i, 1) }));
            }
        }
    };

    inline static MeshConstructor<CubeNormlessCreator> CubeNormless {};
}
