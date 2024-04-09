#pragma once
#include "Mesh.h"
#include "MeshConstructor.h"

namespace Graphics::MeshUtils {
    struct QuadCreator;

    template <> struct OptionsFor<QuadCreator> {};

    struct QuadCreator : MeshConstructor<QuadCreator> {
        using MData = VertexBuilder::MeshConstructData2D;
        QuadCreator(Options) {}

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            std::vector<T> vert;
            vert.reserve(4);
            vert.emplace_back(f(MData { .Position = { +1, +1 }, .VertexIndex = 0 }));
            vert.emplace_back(f(MData { .Position = { +1, -1 }, .VertexIndex = 1 }));
            vert.emplace_back(f(MData { .Position = { -1, +1 }, .VertexIndex = 2 }));
            vert.emplace_back(f(MData { .Position = { -1, -1 }, .VertexIndex = 3 }));

            return { std::move(vert), std::vector<TriIndices> { { 0, 2, 1 }, { 1, 2, 3 } } };
        }
    };

    inline static MeshConstructor<QuadCreator> Quad {};
} // Graphics::MeshUtils
