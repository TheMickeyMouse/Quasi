#pragma once

#include "Constants.h"
#include "Mesh.h"
#include "MeshConstructor.h"
#include "NumTypes.h"

namespace Graphics::MeshUtils {
    struct IcosphereCreator;

    template <> struct OptionsFor<IcosphereCreator> {
        uint subdivisions;
    };

    struct IcosphereCreator : MeshConstructor<IcosphereCreator> {
        using MData = VertexBuilder::MeshConstructData3D;

        IcosphereCreator(const Options& options) : divisions(1 << options.subdivisions) {}
        

#pragma region Icosahedron Data & Utils
        static constexpr uint CORNER_COUNT = 12, EDGE_COUNT = 30, CENTER_COUNT = 20;
        static constexpr float NLEN = 1.90211'30325'90307f, // precomputed sqrt(1 + phi * phi)
                               I_1 = 1.0F / NLEN, I_PHI = Maths::PHI / NLEN;

        static Maths::fvec3 IcoVert(uint idx) {
            // from https://en.wikipedia.org/wiki/Regular_icosahedron#Construction
            switch (idx) {
                case  0: return { 0, +I_1, +I_PHI };
                case  1: return { 0, -I_1, +I_PHI };
                case  2: return { 0, +I_1, -I_PHI };
                case  3: return { 0, -I_1, -I_PHI };
                case  4: return { +I_1, +I_PHI, 0 };
                case  5: return { -I_1, +I_PHI, 0 };
                case  6: return { +I_1, -I_PHI, 0 };
                case  7: return { -I_1, -I_PHI, 0 };
                case  8: return { +I_PHI, 0, +I_1 };
                case  9: return { +I_PHI, 0, -I_1 };
                case 10: return { -I_PHI, 0, +I_1 };
                case 11: return { -I_PHI, 0, -I_1 };
                default: return NAN;
            }
        }

        struct BytePair {
            byte data;
            BytePair(uint top, uint bottom) : data((byte)((top << 4) | bottom)) {}
        };

        inline static const std::array<BytePair, EDGE_COUNT> ICO_EDGES {
            BytePair { 0u,  1u }, BytePair { 0u,  4u }, BytePair { 0u,  5u }, BytePair { 0u,  8u },
            BytePair { 0u, 10u }, BytePair { 1u,  6u }, BytePair { 1u,  7u }, BytePair { 1u,  8u },
            BytePair { 1u, 10u }, BytePair { 2u,  3u }, BytePair { 2u,  4u }, BytePair { 2u,  5u },
            BytePair { 2u,  9u }, BytePair { 2u, 11u }, BytePair { 3u,  6u }, BytePair { 3u,  7u },
            BytePair { 3u,  9u }, BytePair { 3u, 11u }, BytePair { 4u,  5u }, BytePair { 4u,  8u },
            BytePair { 4u,  9u }, BytePair { 5u, 10u }, BytePair { 5u, 11u }, BytePair { 6u,  7u },
            BytePair { 6u,  8u }, BytePair { 6u,  9u }, BytePair { 7u, 10u }, BytePair { 7u, 11u },
            BytePair { 8u,  9u }, BytePair { 10u, 11u }
        };

        static uint FindEdge(BytePair p) {
            return (uint)(std::ranges::lower_bound(ICO_EDGES, p.data, std::less {}, &BytePair::data) - ICO_EDGES.begin());
        };

        static constexpr std::array<TriIndices, CENTER_COUNT> ICO_FACES { {
            { 0, 1, 8 },  { 0, 1, 10 }, { 2, 3, 9 },   { 2, 3, 11 },
            { 4, 8, 9 },  { 6, 8, 9 },  { 5, 10, 11 }, { 7, 10, 11 },
            { 0, 4, 8 },  { 1, 6, 8 },  { 2, 4, 9 },   { 3, 6, 9 },
            { 0, 5, 10 }, { 1, 7, 10 }, { 2, 5, 11 },  { 3, 7, 11 },
            { 0, 4, 5 },  { 2, 4, 5 },  { 1, 6, 7 },   { 3, 6, 7 },
        } };

        static const TriIndices& FaceAt(uint idx) { return ICO_FACES[idx]; }
        [[nodiscard]] const TriIndices& Face() const { return FaceAt(faceIdx); }

        uint faceIdx = 0;
        uint divisions = 0;
        [[nodiscard]] uint EDGE_V_COUNT() const { return divisions - 1; }
        [[nodiscard]] uint CENTER_V_COUNT() const { return (divisions - 2) * (divisions - 1) / 2; }

        [[nodiscard]] uint EdgeIdx(uint e, uint d) const { return CORNER_COUNT + e * EDGE_V_COUNT() + d; }
        [[nodiscard]] uint FaceIdx(uint f, uint p, uint q) const {
            return CORNER_COUNT + EDGE_COUNT * EDGE_V_COUNT() + f * CENTER_V_COUNT() + (p - 2) * (p - 1) / 2 + (q - 1);
        }

        [[nodiscard]] uint IndexOf(uint p, uint q) const {
            if (p == 0 && q == 0) return Face().a;
            if (p == divisions && q == 0) return Face().b;
            if (p == divisions && q == divisions) return Face().c;
            if (q == 0 || q == p || p == divisions) { // edge
                const bool isSide = p == divisions;
                const uint lineStart = isSide ? Face().b : Face().a,
                           lineEnd = q ? Face().c : Face().b,
                           edgeIdx = FindEdge({ lineStart, lineEnd });
                return EdgeIdx(edgeIdx, (isSide ? q : p) - 1);
            }
            return FaceIdx(faceIdx, p, q);
        }
#pragma endregion

        template <stdu::fn_args<MData> F>
        auto CreateImpl(F&& f) -> Mesh<decltype(f(MData {}))> {
            using T = decltype(f(MData {}));
            using namespace Maths;

            std::vector<T> vert;
            std::vector<TriIndices> inds;

            vert.resize(CORNER_COUNT + EDGE_V_COUNT() * EDGE_COUNT + CENTER_V_COUNT() * CENTER_COUNT);
            inds.reserve(divisions * divisions * CENTER_COUNT);
            for (uint i = 0; i < CORNER_COUNT; ++i)
                vert[i] = f(MData { .Position = IcoVert(i), .Normal = IcoVert(i), .VertexIndex = i });

            for (; faceIdx < CENTER_COUNT; ++faceIdx) {
                // loops through a triangle, p = 1 to prevent the (0,0) coordinate
                for (uint p = 1; p <= divisions; ++p) {
                    for (uint q = 0; q <= p; ++q) {
                        if (p == divisions && (q == 0 || q == divisions)) continue;
                        if (q == 0 || q == p || p == divisions) { // edge, interpolate between 2 points
                            const bool isSide = p == divisions;

                            const uint lineStart = isSide ? Face().b : Face().a,
                                       lineEnd = q ? Face().c : Face().b,
                                       edgeIdx = FindEdge({ lineStart, lineEnd }),
                                       dist = isSide ? q : p,
                                       idx = EdgeIdx(edgeIdx, dist - 1);

                            const float factor = (float)dist / (float)divisions;
                            const fvec3 pos = (IcoVert(lineStart) * (1.0f - factor) + IcoVert(lineEnd) * factor).norm();

                            vert[idx] = f(MData { .Position = pos, .Normal = pos, .VertexIndex = idx });
                            continue;
                        }
                        // center, interpolate between 3 points
                        const fvec3 pos = (IcoVert(Face().a) * ((float)(divisions - p) / (float)divisions)
                                         + IcoVert(Face().b) * ((float)(p - q)         / (float)divisions)
                                         + IcoVert(Face().c) * ((float)q               / (float)divisions)).norm();
                        const uint idx = IndexOf(p, q);
                        vert[idx] = f(MData { .Position = pos, .Normal = pos, .VertexIndex = idx });
                    }
                }
                // the indices
                const bool iSwap = (faceIdx % 4 == 1 || faceIdx % 4 == 2) ^ (8 <= faceIdx && faceIdx < 12);
                for (uint p = 0; p < divisions; ++p) {
                    for (uint q = 0; q < p; ++q) {
                        const uint p1 = IndexOf(p, q);
                        inds.emplace_back(p1, IndexOf(p + 1, q + iSwap), IndexOf(p + 1, q + !iSwap));
                        inds.emplace_back(p1, IndexOf(p + !iSwap, q + 1), IndexOf(p + iSwap, q + 1));
                    }
                    inds.emplace_back(IndexOf(p, p), IndexOf(p + 1, p + iSwap), IndexOf(p + 1, p + !iSwap));
                }
            }

            return Mesh<T> { std::move(vert), std::move(inds) };
        }
    };

    inline static MeshConstructor<IcosphereCreator> Icosphere {};
}
