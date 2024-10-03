#pragma once
#include <vector>
#include <algorithm>

#include "RenderObject.h"
#include "TriIndices.h"

namespace Quasi::Graphics {
    class GenericMesh;
    
    template <IVertex Vtx>
    class Mesh {
        template <class T2D, class T3D>
        using VersionOf = std::conditional_t<Vtx::DIMENSION == 2, T2D,
                          std::conditional_t<Vtx::DIMENSION == 3, T3D, void>>;
        using Transform      = VersionOf<Math::Transform2D, Math::Transform3D>;
        using Vector         = VersionOf<Math::fVector2,    Math::fVector3>;
        using Rotation       = VersionOf<Math::fComplex,    Math::Quaternion>;
        using RotationAngles = VersionOf<float,             const Math::fVector3&>;

    public:
        Vec<Vtx> vertices;
        Vec<TriIndices> indices;
        Transform modelTransform;
    public:
        Mesh() = default;
        Mesh(Vec<Vtx> v, Vec<TriIndices> i)
            : vertices(std::move(v)), indices(std::move(i)) {}

        void SetTransform(const Transform& model) { modelTransform = model; }
        void ApplyTransform(const Transform& model) { modelTransform.Apply(model); }
        Mesh& EmbedTransform();
        Mesh& Move  (const Vector& offset) { modelTransform.Translate(offset); return *this; }
        Mesh& Scale (const Vector& scale)  { modelTransform.Scale(scale);      return *this; }
        Mesh& Rotate(RotationAngles rot)   { modelTransform.Rotate(rot);       return *this; }
        Mesh& Rotate(const Rotation& rot)  { modelTransform.Rotate(rot);       return *this; }

        void AddTo(RenderData& rd) const;

        void PushVertex(const Vtx& v) { vertices.push_back(v); }
        void PushIndex(TriIndices i) { indices.push_back(i); }
        void PushVertices(const CollectionOf<Vtx> auto& vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushVertices(IList<Vtx> vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushIndices(const CollectionOf<TriIndices> auto& is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushIndices(IList<TriIndices> is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushTriangle(const Vtx& a, const Vtx& b, const Vtx& c) {
            PushIndex({ vertices.size(), vertices.size() + 1, vertices.size() + 2 });
            PushVertex(a); PushVertex(b); PushVertex(c);
        }
        void PushPolygon(const CollectionOf<Vtx> auto& vs) {
            const u32 begin = vertices.size();
            PushVertices(vs);
            for (u32 i = begin + 1; i < vertices.size() - 1; ++i) PushIndex({ begin, i, i + 1 });
        }
        void PushPolygon(IList<Vtx> vs) { PushPolygon(Span { vs }); }

        struct BatchProxy {
            u32 iOffset;
            Mesh& mesh;

            void PushV(const Vtx& v) { mesh.PushVertex(v); }
            void ReserveV(u32 amount) { mesh.vertices.resize(mesh.vertices.size() + amount); }
            Vtx& VertAt(u32 i) { return mesh.vertices[iOffset + i]; }
            void PushVs(const CollectionOf<Vtx> auto& vs) { mesh.PushVertices(vs); }
            void PushVs(IList<Vtx> vs) { mesh.PushVertices(vs); }

            void PushI(TriIndices i) { mesh.PushIndex(i + iOffset); }
            void PushI(u32 i, u32 j, u32 k) { mesh.PushIndex({ i + iOffset, j + iOffset, k + iOffset }); }
            void ReserveI(u32 amount) { mesh.indices.resize(mesh.indices.size() + amount); }
            void PushIs(const CollectionOf<TriIndices> auto& is) { mesh.PushIndices(is); }
            void PushIs(IList<TriIndices> is) { mesh.PushIndices(is); }

            void PushTri(const Vtx& a, const Vtx& b, const Vtx& c) {
                PushV(a); PushV(b); PushV(c);
                const u32 i = (u32)mesh.vertices.size();
                mesh.PushIndex({ i - 3, i - 2, i - 1 });
            }

            void PushSpan(CollectionOf<Vtx> auto&& vs, CollectionOf<TriIndices> auto&& is) {
                for (const auto& v : vs) PushV(v);
                for (const auto& i : is) PushI(i);
            }

            void PushPolygon(const CollectionOf<Vtx> auto& vs) { mesh.PushPolygon(vs); }
            void PushPolygon(IList<Vtx> vs) { PushPolygon(Span { vs }); }
        };

        BatchProxy NewBatch() { return { (u32)vertices.size(), *this }; }

        // moving isnt that efficient bc vertexes are easy to copy
        Mesh& Add(const Mesh& m);
        static Mesh Combine(IList<Mesh> meshes);

        template <Fn<void, Ref<Vtx>[]> G>
        void GeometryPassTris(G&& g);

        template <Fn<void, Ref<Vtx>> G>
        void GeometryPass(G&& g);

        template <IVertex U, Fn<void, typename Mesh<U>::BatchProxy, Ref<const Vtx>[]> G>
        Mesh<U> GeometryMapTris(G&& g) const;

        template <IVertex U, Fn<U, Ref<const Vtx>> G>
        Mesh<U> GeometryMap(G&& g) const;

        void Clear();

        friend class GraphicsDevice;
        friend class RenderData;
        friend class GenericMesh;
    };
}

#include "Mesh.tpp"