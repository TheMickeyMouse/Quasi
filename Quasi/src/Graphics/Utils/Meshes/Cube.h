#pragma once

#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CubeCreator;

    template <> struct OptionsFor<CubeCreator> {
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct CubeCreator : MeshConstructor<CubeCreator> {
        CubeCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            for (Math::Direction3D i = Math::Direction3D::RIGHT; i < 6; ++i) {
                auto meshp = mesh.NewBatch();

                const u32 ii = (u32)i;
                meshp.PushI(0, 1, 2);
                meshp.PushI(1, 2, 3);

                const Math::fVector3 norm { i };
                const u32 n = ii / 2, flip = (ii & 1) << n;
                const u32 t1 = 0, t2 = n == 0 ? 2 : 1, t3 = n == 2 ? 2 : 4, t4 = t2 + t3;
                meshp.PushV(f(MData { .Position = Math::fVector3 { (Math::Corner3D)(t1 ^ flip) }, .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3 { (Math::Corner3D)(t2 ^ flip) }, .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3 { (Math::Corner3D)(t3 ^ flip) }, .Normal = norm }));
                meshp.PushV(f(MData { .Position = Math::fVector3 { (Math::Corner3D)(t4 ^ flip) }, .Normal = norm }));
            }
        }
    };

    inline static MeshConstructor<CubeCreator> Cube {};
}
