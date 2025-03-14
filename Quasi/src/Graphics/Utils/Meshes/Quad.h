#pragma once

#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct QuadCreator;

    template <> struct OptionsFor<QuadCreator> {
        using MData = VertexBuilder::MeshConstructData2D;
    };

    struct QuadCreator : MeshConstructor<QuadCreator> {
        QuadCreator(Options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& out) {
            auto meshp = out.NewBatch();

            meshp.PushV(f(MData { .Position = { +1, +1 } }));
            meshp.PushV(f(MData { .Position = { +1, -1 } }));
            meshp.PushV(f(MData { .Position = { -1, +1 } }));
            meshp.PushV(f(MData { .Position = { -1, -1 } }));
            meshp.PushI(0, 2, 1);
            meshp.PushI(1, 2, 3);
        }
    };

    inline static MeshConstructor<QuadCreator> Quad {};
} // Graphics::MeshUtils
