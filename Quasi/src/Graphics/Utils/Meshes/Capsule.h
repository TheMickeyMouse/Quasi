#pragma once

#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"

namespace Quasi::Graphics::MeshUtils {
    struct CapsuleCreator;

    template <>
    struct OptionsFor<CapsuleCreator> {
        Math::fVector3 start, end;
        float radius;
        Math::uVector2 sections;

        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct CapsuleCreator : MeshConstructor<CapsuleCreator> {
        Options opt;
        CapsuleCreator(const Options& options) : opt(options) {}

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            using namespace Math;
            const float LATITUDE_SECT = HALF_PI / (float)opt.sections.y, LONGITUDE_SECT = TAU / (float)opt.sections.x;

            const fVector3 Y = (opt.start - opt.end).norm();
            // the { y, -x, 0 } is absolutely arbitrary, just need a nonzero vec thats also not the x axis
            const fVector3 X = fVector3 { Y.y, -Y.x, 0 }.norm();
            const fVector3 Z = Y.cross(X);

            const auto sphereCoord = [&] (float yaw, float pitch) {
                return std::sin(yaw) * std::cos(pitch) * X +
                       std::sin(pitch)                 * Y +
                       std::cos(yaw) * std::cos(pitch) * Z;
            };

            for (u32 hemisphere = 0; hemisphere < 2; ++hemisphere) { // 0 is top, 1 is bottom
                auto meshp = mesh.NewBatch();

                const bool isTop = hemisphere == 0;
                const float sign = isTop ? 1 : -1;
                meshp.PushV(f(MData {
                    .Position = opt.start + opt.radius * sign * Y,
                    .Normal   = sign * Y
                }));
                for (u32 y = 1; y <= opt.sections.y; ++y) {
                    for (u32 x = 0; x < opt.sections.x; ++x) {
                        const fVector3 direction = sign * sphereCoord((float)x * LONGITUDE_SECT, (float)y * LATITUDE_SECT);
                        meshp.PushV(f(MData {
                            .Position = (isTop ? opt.start : opt.end) + direction * opt.radius,
                            .Normal   = direction
                        }));
                        const u32 px = x ? x : opt.sections.x;
                        if (y == 1) {
                            meshp.PushI(0, px, x + 1);
                        } else {
                            const u32 currYOff = (y - 1) * opt.sections.x;
                            const u32 nextYOff = y * opt.sections.x;
                            meshp.PushI(currYOff + px, nextYOff + px, currYOff + x);
                            meshp.PushI(currYOff + x,  nextYOff + px, nextYOff + x);
                        }
                    }
                }
            }
            const u32 lastLoopIndex = 1 + opt.sections.y * opt.sections.x;

            // loop cut
            for (u32 x = 1; x <= opt.sections.x; ++x) {
                const u32 prevX = x == 1 ? opt.sections.x : x - 1;
                mesh.PushIndex(lastLoopIndex - prevX, 2 * lastLoopIndex - prevX,     lastLoopIndex - x);
                mesh.PushIndex(lastLoopIndex - x,     2 * lastLoopIndex - prevX, 2 * lastLoopIndex - x);
            }
        }
    };

    inline static MeshConstructor<CapsuleCreator> Capsule {};
}
