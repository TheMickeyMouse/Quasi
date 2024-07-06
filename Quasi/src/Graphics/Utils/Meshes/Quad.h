#pragma once
#include "Mesh.h"
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
            const u32 iOffset = out.vertices.size();

            out.vertices.emplace_back(f(MData { .Position = { +1, +1 } }));
            out.vertices.emplace_back(f(MData { .Position = { +1, -1 } }));
            out.vertices.emplace_back(f(MData { .Position = { -1, +1 } }));
            out.vertices.emplace_back(f(MData { .Position = { -1, -1 } }));
            out.indices.emplace_back(iOffset + 0, iOffset + 2, iOffset + 1);
            out.indices.emplace_back(iOffset + 1, iOffset + 2, iOffset + 3);
        }
    };

    inline static MeshConstructor<QuadCreator> Quad {};
} // Graphics::MeshUtils
