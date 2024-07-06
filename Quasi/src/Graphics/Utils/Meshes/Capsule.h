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

            auto& vert = mesh.vertices;
            auto& ind = mesh.indices;
            u32 iOffset = vert.size();

            const fVector3 Y = (opt.start - opt.end).norm();
            // the { y, -x, 0 } is absolutely arbitrary, just need a nonzero vec thats also not the x axis
            const fVector3 X = fVector3 { Y.y, -Y.x, 0 }.norm();
            const fVector3 Z = Y.cross(X);

            const auto sphereCoord = [&] (float yaw, float pitch) {
                return std::sin(yaw) * std::cos(pitch) * X +
                       std::sin(pitch)                 * Y +
                       std::cos(yaw) * std::cos(pitch) * Z;
            };

            u32 lastLoopIndex = 0;
            for (u32 hemisphere = 0; hemisphere < 2; ++hemisphere) { // 0 is top, 1 is bottom
                const bool isTop = hemisphere == 0;
                const float sign = isTop ? 1 : -1;
                vert.emplace_back(f(MData {
                    .Position = opt.start + opt.radius * sign * Y,
                    .Normal   = sign * Y
                }));
                for (u32 y = 1; y <= opt.sections.y; ++y) {
                    for (u32 x = 0; x < opt.sections.x; ++x) {
                        const fVector3 direction = sign * sphereCoord((float)x * LONGITUDE_SECT, (float)y * LATITUDE_SECT);
                        vert.emplace_back(f(MData {
                            .Position = (isTop ? opt.start : opt.end) + direction * opt.radius,
                            .Normal   = direction
                        }));
                        const u32 px = x ? x : opt.sections.x;
                        if (y == 1) {
                            ind.emplace_back(iOffset, iOffset + px, iOffset + x + 1);
                        } else {
                            const u32 currYOff = iOffset + (y - 1) * opt.sections.x;
                            const u32 nextYOff = iOffset + y * opt.sections.x;
                            ind.emplace_back(currYOff + px, nextYOff + px, currYOff + x);
                            ind.emplace_back(currYOff + x,  nextYOff + px, nextYOff + x);
                        }
                    }
                }
                lastLoopIndex = vert.size();
                iOffset = vert.size();
            }
            iOffset = vert.size();

            // loop cut
            for (u32 x = 1; x <= opt.sections.x; ++x) {
                const u32 prevX = x == 1 ? opt.sections.x : x - 1;
                ind.emplace_back(lastLoopIndex - prevX, iOffset - prevX, lastLoopIndex - x);
                ind.emplace_back(lastLoopIndex - x,     iOffset - prevX, iOffset       - x);
            }
        }
    };

    inline static MeshConstructor<CapsuleCreator> Capsule {};
}
