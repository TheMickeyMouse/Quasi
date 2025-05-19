#pragma once

#include "MeshConstructor.h"
#include "Mesh.h"
#include "Math/Vector.h"
#include "Utils/Algorithm.h"

namespace Quasi::Graphics {
    struct TriIndices;
}

namespace Quasi::Graphics::VertexBuilder {
    struct MeshConstructData3D;
}

namespace Quasi::Graphics::MeshUtils {
    struct IcosphereCreator;

    template <> struct OptionsFor<IcosphereCreator> {
        u32 subdivisions;
        using MData = VertexBuilder::MeshConstructData3D;
    };

    struct IcosphereCreator : MeshConstructor<IcosphereCreator> {
        IcosphereCreator(const Options& options) : divisions(1 << options.subdivisions) {}

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

        struct BytePair {
            byte data;
            BytePair(u32 top, u32 bottom) : data((byte)((top << 4) | bottom)) {}
            Comparison operator<=>(const BytePair& pair) const { return Cmp::Between(data, pair.data); }
        };

        inline static const Array<BytePair, EDGE_COUNT> ICO_EDGES {
            BytePair { 0,  1 }, BytePair { 0,  4 }, BytePair { 0,  5 }, BytePair { 0,  8 },
            BytePair { 0, 10 }, BytePair { 1,  6 }, BytePair { 1,  7 }, BytePair { 1,  8 },
            BytePair { 1, 10 }, BytePair { 2,  3 }, BytePair { 2,  4 }, BytePair { 2,  5 },
            BytePair { 2,  9 }, BytePair { 2, 11 }, BytePair { 3,  6 }, BytePair { 3,  7 },
            BytePair { 3,  9 }, BytePair { 3, 11 }, BytePair { 4,  5 }, BytePair { 4,  8 },
            BytePair { 4,  9 }, BytePair { 5, 10 }, BytePair { 5, 11 }, BytePair { 6,  7 },
            BytePair { 6,  8 }, BytePair { 6,  9 }, BytePair { 7, 10 }, BytePair { 7, 11 },
            BytePair { 8,  9 }, BytePair { 10, 11 }
        };

        static u32 FindEdge(BytePair p) {
            return ICO_EDGES.LowerBound(p);
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
                          edgeIdx = FindEdge({ lineStart, lineEnd });
                return EdgeIdx(edgeIdx, (isSide ? q : p) - 1);
            }
            return FaceIdx(faceIdx, p, q);
        }
#pragma endregion

        template <FnArgs<MData> F>
        void MergeImpl(F&& f, Mesh<ResultingV<F>>& mesh) {
            using namespace Math;

            auto meshp = mesh.NewBatch();
            meshp.ReserveV(CORNER_COUNT + EDGE_V_COUNT() * EDGE_COUNT + CENTER_V_COUNT() * CENTER_COUNT);

            for (u32 i = 0; i < CORNER_COUNT; ++i)
                meshp.VertAt(i) = f(MData { .Position = IcoVert[i], .Normal = IcoVert[i] });

            for (; faceIdx < CENTER_COUNT; ++faceIdx) {
                // loops through a triangle, p = 1 to prevent the (0,0) coordinate
                for (u32 p = 1; p <= divisions; ++p) {
                    for (u32 q = 0; q <= p; ++q) {
                        if (p == divisions && (q == 0 || q == divisions)) continue;
                        if (q == 0 || q == p || p == divisions) { // edge, interpolate between 2 points
                            const bool isSide = p == divisions;

                            const u32 lineStart = isSide ? Face().j : Face().i,
                                      lineEnd = q ? Face().k : Face().j,
                                      edgeIdx = FindEdge({ lineStart, lineEnd }),
                                      dist = isSide ? q : p,
                                      idx = EdgeIdx(edgeIdx, dist - 1);

                            const float factor = (float)dist / (float)divisions;
                            const fv3 pos = (IcoVert[lineStart] * (1.0f - factor) + IcoVert[lineEnd] * factor).Norm();

                            meshp.VertAt(idx) = f(MData { .Position = pos, .Normal = pos });
                            continue;
                        }
                        // center, interpolate between 3 points
                        const fv3 pos = (IcoVert[Face().i] * ((float)(divisions - p) / (float)divisions)
                                            + IcoVert[Face().j] * ((float)(p - q)         / (float)divisions)
                                            + IcoVert[Face().k] * ((float)q               / (float)divisions)).Norm();
                        const u32 idx = IndexOf(p, q);
                        meshp.VertAt(idx) = f(MData { .Position = pos, .Normal = pos });
                    }
                }
                // the indices
                const bool iSwap = (faceIdx % 4 == 1 || faceIdx % 4 == 2) ^ (8 <= faceIdx && faceIdx < 12);
                for (u32 p = 0; p < divisions; ++p) {
                    for (u32 q = 0; q < p; ++q) {
                        const u32 p1 = IndexOf(p, q);
                        meshp.PushI(p1, IndexOf(p + 1, q + iSwap),  IndexOf(p + 1, q + !iSwap));
                        meshp.PushI(p1, IndexOf(p + !iSwap, q + 1), IndexOf(p + iSwap, q + 1));
                    }
                    meshp.PushI(IndexOf(p, p), IndexOf(p + 1, p + iSwap), IndexOf(p + 1, p + !iSwap));
                }
            }
        }
    };

    inline static MeshConstructor<IcosphereCreator> Icosphere {};
}
