#pragma once
#include "Circle.h"
#include "MeshBuilder.h"

namespace Quasi::Graphics::Meshes {
    struct Arrow3D : IMeshBuilder3D<Arrow3D> {
        Math::fv3 start, end;
        float scale = 1.0f;
        u32 subdivisions = 3;
        Arrow3D(const Math::fv3& start, const Math::fv3& end, float scale = 1, u32 sub = 3)
            : start(start), end(end), scale(scale), subdivisions(sub) {}

        static constexpr float DEFAULT_TIP_SIZE = 0.18f,
                               MAX_TIP_SIZE = 0.13f,
                               THICKNESS_FACTOR = 0.4f,
                               INV_NORMAL_LEN = 2 / (float)Math::ROOT_5;

        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, VertexNormal3D>> auto&& mesh) {
            using namespace Math;
            const auto [fwd, len] = (end - start).NormAndLen();
            const float tipSize = std::max(len * MAX_TIP_SIZE, scale * DEFAULT_TIP_SIZE),
                        cylRadius = tipSize * THICKNESS_FACTOR;

            const fv3 beforeTip = end - fwd * (tipSize * 2);
            fv3 planeX = fwd.AnyTangent();

            const u32 divisions = 3 << subdivisions;
            {
                const Rotor2D halfRot = Radians(f32s::Comp(-(int)subdivisions, PI / 3.0f));
                const Matrix3x3 step = Quaternion { halfRot.Cos(), fwd * halfRot.Sin() }.AsMatrixLinear();
                fv3 p = planeX * INV_NORMAL_LEN;
                const fv3 halfFwd = fwd * (0.5f * INV_NORMAL_LEN);
                const float normalToPos = tipSize * (ROOT_5 * 0.5f), middleInv = 0.5f / halfRot.Cos();
                const u32 divisionsTimes4 = divisions * 4;
                for (u32 i = 0; i < divisionsTimes4; i += 4) {
                    const u32 w = i == 0 ? divisionsTimes4 : i;
                    const fv3 r = step * p, n = (p + r) * middleInv;
                    mesh.PushV(f(VertexNormal3D { end,        n + halfFwd }));
                    mesh.PushV(f(VertexNormal3D { beforeTip + p * normalToPos, p + halfFwd }));
                    mesh.PushI(i, i + 1, w - 3);
                    const fv3 qn = p * (1 / INV_NORMAL_LEN), q = qn * cylRadius;
                    mesh.PushV(f(VertexNormal3D { beforeTip + q, qn }));
                    mesh.PushV(f(VertexNormal3D { start     + q, qn }));
                    mesh.PushI(i + 2, i + 3, w - 2);
                    mesh.PushI(i + 3, w - 1, w - 2);
                    p = r;
                }
            }

            mesh = mesh.NewBatch();
            planeX *= tipSize;
            const fv3 n = -fwd;
            Rotor3D step = Rotor3D::FromTrig(0.5f, HALF_ROOT_3, fwd);
            {
                const fv3 planeX120 = step.Rotate(planeX),
                          cylX120 = planeX120 * THICKNESS_FACTOR,
                          cylX = planeX * THICKNESS_FACTOR;
                mesh.PushV(f(VertexNormal3D { beforeTip + planeX120,            n }));
                mesh.PushV(f(VertexNormal3D { start     + cylX120,              n }));
                mesh.PushV(f(VertexNormal3D { beforeTip + planeX,               n }));
                mesh.PushV(f(VertexNormal3D { start     + cylX,                 n }));
                mesh.PushV(f(VertexNormal3D { beforeTip - (planeX + planeX120), n }));
                mesh.PushV(f(VertexNormal3D { start     - (cylX + cylX120),     n }));

                mesh.PushI(2, 0, 4);
                mesh.PushI(3, 1, 5);
            }

            for (u32 i = 0; i < subdivisions; ++i) {
                const Rotor3D half = step.Halved();
                fv3 vertex = half * planeX;

                u32 p = 2, q = (6 << i) + 1;
                for (u32 j = 0; j < (3 << i); ++j) {
                    mesh.PushV(f(VertexNormal3D { beforeTip + vertex, n }));
                    mesh.PushV(f(VertexNormal3D { start     + vertex * THICKNESS_FACTOR, n }));
                    vertex = step * vertex;

                    const u32 r = q >> u32s::CountRightOnes(q);
                    mesh.PushI(p,     q - 1, r);
                    mesh.PushI(p + 1, q,     r + 1);
                    p = r;
                    q += 2;
                }
                step = half;
            }
        }
    };
}
