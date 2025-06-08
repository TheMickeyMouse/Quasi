#pragma once

#include "MeshBuilder.h"
#include "Mesh.h"
#include "Math/Vector.h"
#include "Utils/Algorithm.h"

namespace Quasi::Graphics::Meshes {
    struct Icosphere : IMeshBuilder3D<Icosphere> {
        float radius = 1.0f;
        Icosphere(float r = 1, u32 subdivisions = 3) : radius(r), divisions(1 << subdivisions) {}

#pragma region Icosahedron Data & Utils
        static constexpr u32 CORNER_COUNT = 12, EDGE_COUNT = 30, CENTER_COUNT = 20;
        static constexpr float NLEN = 1.90211'30325'90307f, // precomputed sqrt(1 + phi * phi)
                               I_1 = 1.0F / NLEN, I_PHI = Math::PHI / NLEN;

        inline static const Array<Math::fv3, CORNER_COUNT> IcoVert = {
            // from https://en.wikipedia.org/wiki/Regular_icosahedron#Construction
            Math::fv3 { 0, +I_1, +I_PHI },
            Math::fv3 { 0, -I_1, +I_PHI },
            Math::fv3 { 0, +I_1, -I_PHI },
            Math::fv3 { 0, -I_1, -I_PHI },
            Math::fv3 { +I_1, +I_PHI, 0 },
            Math::fv3 { -I_1, +I_PHI, 0 },
            Math::fv3 { +I_1, -I_PHI, 0 },
            Math::fv3 { -I_1, -I_PHI, 0 },
            Math::fv3 { +I_PHI, 0, +I_1 },
            Math::fv3 { +I_PHI, 0, -I_1 },
            Math::fv3 { -I_PHI, 0, +I_1 },
            Math::fv3 { -I_PHI, 0, -I_1 },
        };

        static constexpr u8 ICO_EDGES[EDGE_COUNT] {
            0x01, 0x04, 0x05, 0x08, 0x0A, 0x16,
            0x17, 0x18, 0x1A, 0x23, 0x24, 0x25,
            0x29, 0x2B, 0x36, 0x37, 0x39, 0x3B,
            0x45, 0x48, 0x49, 0x5A, 0x5B, 0x67,
            0x68, 0x69, 0x7A, 0x7B, 0x89, 0xAB,
        };

        static u32 FindEdge(u8 p) {
            return Span { ICO_EDGES }.LowerBound(p);
        };

        static constexpr Array<TriIndices, CENTER_COUNT> ICO_FACES { {
            { 0, 1, 8 },  { 0, 1, 10 }, { 2, 3,  9 },   { 2, 3,  11 },
            { 4, 8, 9 },  { 6, 8, 9 },  { 5, 10, 11 },  { 7, 10, 11 },
            { 0, 4, 8 },  { 1, 6, 8 },  { 2, 4,  9 },   { 3, 6,  9 },
            { 0, 5, 10 }, { 1, 7, 10 }, { 2, 5,  11 },  { 3, 7,  11 },
            { 0, 4, 5 },  { 2, 4, 5 },  { 1, 6,  7 },   { 3, 6,  7 },
        } };
        const TriIndices& Face() const { return ICO_FACES[faceIdx]; }

        u32 faceIdx = 0;
        u32 divisions = 0;
        u32 EDGE_V_COUNT() const { return divisions - 1; }
        u32 CENTER_V_COUNT() const { return (divisions - 2) * (divisions - 1) / 2; }

        u32 EdgeIdx(u32 e, u32 d) const { return CORNER_COUNT + e * EDGE_V_COUNT() + d; }
        u32 FaceIdx(u32 f, u32 p, u32 q) const {
            return CORNER_COUNT + EDGE_COUNT * EDGE_V_COUNT() + f * CENTER_V_COUNT() + (p - 2) * (p - 1) / 2 + (q - 1);
        }

        u32 IndexOf(u32 p, u32 q) const {
            if (p == 0 && q == 0) return Face().i;
            if (p == divisions && q == 0) return Face().j;
            if (p == divisions && q == divisions) return Face().k;
            if (q == 0 || q == p || p == divisions) { // edge
                const bool isSide = p == divisions;
                const u32 lineStart = isSide ? Face().j : Face().i,
                          lineEnd = q ? Face().k : Face().j,
                          edgeIdx = FindEdge((lineStart << 4) + lineEnd);
                return EdgeIdx(edgeIdx, (isSide ? q : p) - 1);
            }
            return FaceIdx(faceIdx, p, q);
        }
#pragma endregion

        template <FnArgs<VertexNormal3D> F>
        void MergeImpl(F&& f, IMeshBatch<FuncResult<F, VertexNormal3D>> auto&& mesh) {
            using namespace Math;

            mesh.ResizeV(CORNER_COUNT + EDGE_V_COUNT() * EDGE_COUNT + CENTER_V_COUNT() * CENTER_COUNT);

            for (u32 i = 0; i < CORNER_COUNT; ++i)
                mesh.VertAt(i) = f(VertexNormal3D { .Position = IcoVert[i] * radius, .Normal = IcoVert[i] });

            for (; faceIdx < CENTER_COUNT; ++faceIdx) {
                // loops through a triangle, p = 1 to prevent the (0,0) coordinate
                for (u32 p = 1; p <= divisions; ++p) {
                    for (u32 q = 0; q <= p; ++q) {
                        if (p == divisions && (q == 0 || q == divisions)) continue;
                        if (q == 0 || q == p || p == divisions) { // edge, interpolate between 2 points
                            const bool isSide = p == divisions;

                            const u32 lineStart = isSide ? Face().j : Face().i,
                                      lineEnd = q ? Face().k : Face().j,
                                      edgeIdx = FindEdge((lineStart << 4) + lineEnd),
                                      dist = isSide ? q : p,
                                      idx = EdgeIdx(edgeIdx, dist - 1);

                            const float factor = (float)dist / (float)divisions;
                            const fv3 pos = (IcoVert[lineStart] * (1.0f - factor) + IcoVert[lineEnd] * factor).Norm();

                            mesh.VertAt(idx) = f(VertexNormal3D { .Position = radius * pos, .Normal = pos });
                            continue;
                        }
                        // center, interpolate between 3 points
                        const fv3 pos = (IcoVert[Face().i] * ((float)(divisions - p) / (float)divisions)
                                       + IcoVert[Face().j] * ((float)(p - q)         / (float)divisions)
                                       + IcoVert[Face().k] * ((float)q               / (float)divisions)).Norm();
                        const u32 idx = IndexOf(p, q);
                        mesh.VertAt(idx) = f(VertexNormal3D { .Position = radius * pos, .Normal = pos });
                    }
                }
                // the indices
                const bool iSwap = (faceIdx % 4 == 1 || faceIdx % 4 == 2) ^ (8 <= faceIdx && faceIdx < 12);
                for (u32 p = 0; p < divisions; ++p) {
                    for (u32 q = 0; q < p; ++q) {
                        const u32 p1 = IndexOf(p, q);
                        mesh.PushI(p1, IndexOf(p + 1, q + !iSwap), IndexOf(p + 1, q + iSwap));
                        mesh.PushI(p1, IndexOf(p + iSwap, q + 1),  IndexOf(p + !iSwap, q + 1));
                    }
                    mesh.PushI(IndexOf(p, p), IndexOf(p + 1, p + !iSwap), IndexOf(p + 1, p + iSwap));
                }
            }
        }
    };
}
