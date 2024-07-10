#pragma once

#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct SphereCreator;

    template <> struct OptionsFor<SphereCreator> {
        Math::uVector2 sections;
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct SphereCreator : MeshConstructor<SphereCreator> {
        Options opt;

        SphereCreator(const Options& opt) : opt(opt) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            using namespace Math;
            const float LATITUDE_SECT = PI / (float)opt.sections.y, LONGITUDE_SECT = TAU / (float)opt.sections.x;

            auto meshp = mesh.NewBatch();
            meshp.PushV(f(MData { .Position = { 0, 1, 0 }, .Normal = { 0, 1, 0 } }));
            for (u32 lat = 0; lat < opt.sections.y; ++lat) {
                for (u32 lon = 0; lon < opt.sections.x; ++lon) {
                    if (lat != 0) {
                        const float latAngle = HALF_PI - LATITUDE_SECT * (float)lat,
                                    lonAngle = LONGITUDE_SECT * (float)lon;
                        const fVector3 pos = fVector3::from_spheric(1, lonAngle, latAngle);
                        meshp.PushV(f(MData { .Position = pos, .Normal = pos }));
                    }

                    const u32 lon_1 = (lon + 1) % opt.sections.x,
                              off = 1 + (lat - 1) * opt.sections.x,
                              offNext = off + opt.sections.x;
                    if (lat == 0) {
                        meshp.PushI(0, 1 + lon, 1 + lon_1);
                    } else if (lat == opt.sections.y - 1) {
                        meshp.PushI(off + lon_1, off + lon, 1 + opt.sections.x * (opt.sections.y - 1));
                    } else {
                        meshp.PushI(off + lon,   offNext + lon, off     + lon_1);
                        meshp.PushI(off + lon_1, offNext + lon, offNext + lon_1);
                    }
                }
            }
            meshp.PushV(f(MData { .Position = { 0, -1, 0 }, .Normal = { 0, -1, 0 } }));
        }
    };

    inline static MeshConstructor<SphereCreator> Sphere {};
}
