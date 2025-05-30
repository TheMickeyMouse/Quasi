#pragma once

#include "MeshBuilder.h"
#include "Utils/Numeric.h"

namespace Quasi::Graphics::Meshes {
    struct Circle : IMeshBuilder2D<Circle> {
        float radius = 1.0f;
        u32 subdivisions = 4;
        Circle(float r = 1.0f, u32 sub = 4) : radius(r), subdivisions(sub) {}

        static constexpr float PRECOMPUTED_POINTS[] = {
            +1,     0,                 // sub 0
            -0.5f,  Math::HALF_ROOT_3,
            -0.5f, -Math::HALF_ROOT_3,

            +0.5f,  Math::HALF_ROOT_3, // sub 1
            -1,     0,
            +0.5f, -Math::HALF_ROOT_3,

            +Math::HALF_ROOT_3, +0.5f, // sub 2
            0, 1,
            -Math::HALF_ROOT_3, +0.5f,
            -Math::HALF_ROOT_3, -0.5f,
            0, -1,
            +Math::HALF_ROOT_3, -0.5f,

            +0.965925826289f,   +0.258819045103f, // sub 3
            +Math::HALF_ROOT_2, +Math::HALF_ROOT_2,
            +0.258819045103f,   +0.965925826289f,
            -0.258819045103f,   +0.965925826289f,
            -Math::HALF_ROOT_2, +Math::HALF_ROOT_2,
            -0.965925826289f,   +0.258819045103f,
            -0.965925826289f,   -0.258819045103f,
            -Math::HALF_ROOT_2, -Math::HALF_ROOT_2,
            -0.258819045103f,   -0.965925826289f,
            +0.258819045103f,   -0.965925826289f,
            +Math::HALF_ROOT_2, -Math::HALF_ROOT_2,
            +0.965925826289f,   -0.258819045103f,

            +0.991444861374f, +0.130526192220f, // sub 4
            +0.923879532511f, +0.382683432365f,
            +0.793353340291f, +0.608761429009f,
            +0.608761429009f, +0.793353340291f,
            +0.382683432365f, +0.923879532511f,
            +0.130526192220f, +0.991444861374f,
            -0.130526192220f, +0.991444861374f,
            -0.382683432365f, +0.923879532511f,
            -0.608761429009f, +0.793353340291f,
            -0.793353340291f, +0.608761429009f,
            -0.923879532511f, +0.382683432365f,
            -0.991444861374f, +0.130526192220f,
            -0.991444861374f, -0.130526192220f,
            -0.923879532511f, -0.382683432365f,
            -0.793353340291f, -0.608761429009f,
            -0.608761429009f, -0.793353340291f,
            -0.382683432365f, -0.923879532511f,
            -0.130526192220f, -0.991444861374f,
            +0.130526192220f, -0.991444861374f,
            +0.382683432365f, -0.923879532511f,
            +0.608761429009f, -0.793353340291f,
            +0.793353340291f, -0.608761429009f,
            +0.923879532511f, -0.382683432365f,
            +0.991444861374f, -0.130526192220f,
        };

        static const Math::fv2& GetPointBinaryIndex(u32 i, u32 max, u32 bits, const Math::fv2* largerThan4) {
            const u32 j = i | max, depth = j - (j & j - 1);
            if (depth == 0) return *Memory::TransmutePtr<const Math::fv2>(&PRECOMPUTED_POINTS[2 * (i >> bits)]);
            if (depth <= 4)
                // return *Memory::TransmutePtr<const Math::fv2>(&PRECOMPUTED_POINTS[2 * ((3 << (depth - 1)) + (i >> (bits - depth)))]);
                return *Memory::TransmutePtr<const Math::fv2>(&PRECOMPUTED_POINTS[3 * depth + 2 * i / depth]);
            return largerThan4[(depth == 32 ? 0 : 3 * depth) + (i / depth)];
        }

        template <FnArgs<Vertex2D> F>
        void MergeImpl(F&& f, MeshBatch<FuncResult<F, Vertex2D>> mesh) {
            const u32 total = 3 << subdivisions;
            mesh.ReserveV(total);
            mesh.VertAt(0)                 = f(Vertex2D { { radius, 0 } });
            mesh.VertAt(1 << subdivisions) = f(Vertex2D { { -radius * 0.5f,  radius * Math::HALF_ROOT_3 } });
            mesh.VertAt(2 << subdivisions) = f(Vertex2D { { -radius * 0.5f, -radius * Math::HALF_ROOT_3 } });
            mesh.PushI( 0, 1 << subdivisions, 2 << subdivisions );
            const float* pointData = PRECOMPUTED_POINTS;
            for (u32 k = 1; k <= std::min(subdivisions, 4u); ++k) {
                pointData += 3 << k;
                const u32 n = 1 << (subdivisions - k);
                for (usize j = 1; j < (3 << k); j += 2) {
                    const u32 v = (j + 1) << (subdivisions - k);
                    mesh.VertAt(v) = f(Vertex2D { { radius * pointData[j - 1], radius * pointData[j] } });
                    mesh.PushI(v - n, v, (v + n) == total ? 0 : (v + n));
                }
            }
            if (subdivisions <= 4) return;

            Math::Rotor2D step = Math::Rotor2D::FromComplex({ 0.991444861374f, 0.130526192220f });
            for (u32 k = 4; k <= subdivisions; ++k) {
                const u32 n = 1 << (subdivisions - k), skip = n << 1;
                const Math::Rotor2D half = step.Halved();
                Math::Rotor2D r = half;
                for (usize j = n; j < total; j += skip) {
                    mesh.VertAt(j) = f(Vertex2D { radius * r.IHat() });
                    mesh.PushI(j - n, j, (j + n) == total ? 0 : (j + n));
                    r += step;
                }
                step = half;
            }
        }
    };
}
