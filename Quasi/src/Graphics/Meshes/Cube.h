#pragma once

#include "MeshBuilder.h"
#include "Mesh.h"
#include "Utils/Numeric.h"

namespace Quasi::Graphics::Meshes {
    struct Cube : IMeshBuilder3D<Cube> {
        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, MeshBatch<FuncResult<F, VertexNormal3D>> mesh) {
            using namespace Math;
            static constexpr u16 cornerData[6] {
                0b000'010'100'110,
                0b001'101'011'111,
                0b000'100'001'101,
                0b010'011'110'111,
                0b000'001'010'011,
                0b100'110'101'111,
            };
            for (usize i = 0; i < 6; ++i) {
                auto meshp = mesh.NewBatch();
                meshp.PushI(0, 1, 2);
                meshp.PushI(2, 1, 3);

                fv3 n;
                n[i / 2] = i % 2 ? 1 : -1;
                const u32 quad = cornerData[i];
                const bool corners[4][3] {
                    { (quad >> 0 & 1) == 1, (quad >> 1  & 1) == 1, (quad >> 2  & 1) == 1 },
                    { (quad >> 3 & 1) == 1, (quad >> 4  & 1) == 1, (quad >> 5  & 1) == 1 },
                    { (quad >> 6 & 1) == 1, (quad >> 7  & 1) == 1, (quad >> 8  & 1) == 1 },
                    { (quad >> 9 & 1) == 1, (quad >> 10 & 1) == 1, (quad >> 11 & 1) == 1 },
                };
                meshp.PushV(f(VertexNormal3D { .Position = fv3::FromCorner(corners[0]), .Normal = n }));
                meshp.PushV(f(VertexNormal3D { .Position = fv3::FromCorner(corners[1]), .Normal = n }));
                meshp.PushV(f(VertexNormal3D { .Position = fv3::FromCorner(corners[2]), .Normal = n }));
                meshp.PushV(f(VertexNormal3D { .Position = fv3::FromCorner(corners[3]), .Normal = n }));
            }
        }
    };
}
