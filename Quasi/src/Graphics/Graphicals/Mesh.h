#pragma once
#include <algorithm>

#include "RenderObject.h"
#include "TriIndices.h"

namespace Quasi::Graphics {
    class GenericMesh;
    
    template <IVertex Vtx>
    class Mesh {
        template <class T2D, class T3D>
        using VersionOf = IfElse<Vtx::DIMENSION == 2, T2D,
                          IfElse<Vtx::DIMENSION == 3, T3D, void>>;
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
        Mesh(IList<Vtx> v, IList<TriIndices> i) : vertices(Vecs::FromIList(v)), indices(Vecs::FromIList(i)) {}
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

        void PushVertex(const Vtx& v) { vertices.Push(v); }
        void PushIndex(TriIndices i) { indices.Push(i); }
        void PushVertices(const Collection<Vtx> auto& vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushVertices(IList<Vtx> vs) { for (const Vtx& v : vs) PushVertex(v); }
        void PushIndices(const Collection<TriIndices> auto& is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushIndices(IList<TriIndices> is) { for (const TriIndices& i : is) PushIndex(i); }
        void PushTriangle(const Vtx& a, const Vtx& b, const Vtx& c) {
            PushIndex({ vertices.Length(), vertices.Length() + 1, vertices.Length() + 2 });
            PushVertex(a); PushVertex(b); PushVertex(c);
        }
        void PushPolygon(const Collection<Vtx> auto& vs) {
            const u32 begin = vertices.Length();
            PushVertices(vs);
            for (u32 i = begin + 1; i < vertices.Length() - 1; ++i) PushIndex({ begin, i, i + 1 });
        }
        void PushPolygon(IList<Vtx> vs) { PushPolygon(Span { vs }); }

        struct BatchProxy {
            u32 iOffset;
            Mesh& mesh;

            void PushV(const Vtx& v) { mesh.PushVertex(v); }
            void ReserveV(u32 amount) { mesh.vertices.Resize(mesh.vertices.Length() + amount); }
            Vtx& VertAt(u32 i) { return mesh.vertices[iOffset + i]; }
            void PushVs(const Collection<Vtx> auto& vs) { mesh.PushVertices(vs); }
            void PushVs(IList<Vtx> vs) { mesh.PushVertices(vs); }

            void PushI(TriIndices i) { mesh.PushIndex(i + iOffset); }
            void PushI(u32 i, u32 j, u32 k) { mesh.PushIndex({ i + iOffset, j + iOffset, k + iOffset }); }
            void ReserveI(u32 amount) { mesh.indices.Resize(mesh.indices.Length() + amount); }
            void PushIs(const Collection<TriIndices> auto& is) { mesh.PushIndices(is); }
            void PushIs(IList<TriIndices> is) { mesh.PushIndices(is); }

            void PushTri(const Vtx& a, const Vtx& b, const Vtx& c) {
                PushV(a); PushV(b); PushV(c);
                const u32 i = (u32)mesh.vertices.Length();
                mesh.PushIndex({ i - 3, i - 2, i - 1 });
            }

            void PushSpan(const Collection<Vtx> auto& vs, const Collection<TriIndices> auto& is) {
                for (const auto& v : vs) PushV(v);
                for (const auto& i : is) PushI(i);
            }

            void PushPolygon(const Collection<Vtx> auto& vs) { mesh.PushPolygon(vs); }
            void PushPolygon(IList<Vtx> vs) { PushPolygon(Spans::FromIList(vs)); }
        };

        BatchProxy NewBatch() { return { (u32)vertices.Length(), *this }; }

        // moving isnt that efficient bc vertexes are easy to copy
        Mesh& Add(const Mesh& m);
        static Mesh Combine(Span<Mesh> meshes);

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