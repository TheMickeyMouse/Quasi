#pragma once

#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"
#include "NumTypes.h"

namespace Graphics::MeshUtils {
    struct SphereCreator;

    template <> struct OptionsFor<SphereCreator> {
        Maths::uvec2 sections;
    };

    struct SphereCreator : MeshConstructor<SphereCreator> {
        Options opt;
        using MData = VertexBuilder::MeshConstructData3D;

        SphereCreator(const Options& opt) : opt(opt) {}

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            using namespace Maths;
            const float LATITUDE_SECT = PI / (float)opt.sections.y, LONGITUDE_SECT = TAU / (float)opt.sections.x;

            Mesh<T> mesh;
            auto& vert = mesh.GetVertices();
            auto& ind = mesh.GetIndices();
            vert.reserve(2 + (opt.sections.y - 1) * opt.sections.x);
            ind.reserve(2 * (opt.sections.y - 1) * opt.sections.x);

            vert.emplace_back(f(MData { .Position = { 0, 1, 0 }, .Normal = { 0, 1, 0 }, .VertexIndex = 0 }));
            uint x = 0;
            for (uint lat = 0; lat < opt.sections.y; ++lat) {
                for (uint lon = 0; lon < opt.sections.x; ++lon) {
                    if (lat != 0) {
                        const float latAngle = HALF_PI - LATITUDE_SECT * (float)lat,
                                    lonAngle = LONGITUDE_SECT * (float)lon;
                        const fvec3 pos = fvec3::from_spheric(1, lonAngle, latAngle);
                        vert.emplace_back(f(MData { .Position = pos, .Normal = pos, .VertexIndex = ++x }));
                    }

                    const uint lon_1 = (lon + 1) % opt.sections.x,
                               off = 1 + (lat - 1) * opt.sections.x,
                               offNext = off + opt.sections.x;
                    if (lat == 0) {
                        ind.emplace_back(0, 1 + lon, 1 + lon_1);
                    } else if (lat == opt.sections.y - 1) {
                        ind.emplace_back(off + lon_1, off + lon, (uint)(vert.capacity() - 1));
                    } else {
                        ind.emplace_back(off + lon,   offNext + lon, off     + lon_1);
                        ind.emplace_back(off + lon_1, offNext + lon, offNext + lon_1);
                    }
                }
            }
            vert.emplace_back(f(MData { .Position = { 0, -1, 0 }, .Normal = { 0, -1, 0 }, .VertexIndex = x }));
            return mesh;
        }
    };

    inline static MeshConstructor<SphereCreator> Sphere {};
}
