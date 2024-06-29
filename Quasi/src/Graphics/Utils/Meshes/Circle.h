#pragma once
#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CircleCreator;

    template <> struct OptionsFor<CircleCreator> {
        u32 subdivisions;
    };

    struct CircleCreator : MeshConstructor<CircleCreator> {
        Options opt;
        using MData = VertexBuilder::MeshConstructData2D;

        CircleCreator(const Options& opt) : opt(opt) {}

        template <FnArgs<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            const float angle = Math::TAU / (float)opt.subdivisions;
            Mesh<T> mesh;
            mesh.vertices.reserve(opt.subdivisions + 1);
            mesh.indices.reserve(opt.subdivisions);

            mesh.vertices.emplace_back(f(MData { Math::fVector2::ZERO(), 0 }));
            for (uint i = 0; i < opt.subdivisions; ++i) {
                mesh.vertices.push_back(f(MData { Math::fVector2::from_polar(1.0f, angle * (float)i), i + 1 }));
                mesh.indices.emplace_back(0, i + 1, (i + 1) % opt.subdivisions + 1);
            }

            return mesh;
        }
    };

    inline static MeshConstructor<CircleCreator> Circle {};
}
