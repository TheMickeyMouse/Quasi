#pragma once
#include <vector>
#include <algorithm>

#include "RenderObject.h"
#include "TriIndices.h"

namespace Quasi::Graphics {
    namespace Primitives {
        class Quad;
        class Tri;
    }

    class GenericMesh;
    
    template <class T>
    class Mesh {
    public:
        using Vertex = T;
        Vec<Vertex> vertices;
        Vec<TriIndices> indices;
        Math::Matrix3D modelTransform;
    public:
        Mesh() = default;
        Mesh(Vec<Vertex> v, Vec<TriIndices> i)
            : vertices(std::move(v)), indices(std::move(i)) {}

        void SetTransform(const Math::Matrix3D& model) { modelTransform = model; }
        void Transform(const Math::Matrix3D& model) { modelTransform *= model; }
        Mesh& ApplyTransform(const Math::Matrix3D& model);
        Mesh& ApplyTransform() { return ApplyTransform(modelTransform); }
        Mesh& Move(const Math::fVector3& offset) { return ApplyTransform(Math::Matrix3D::translate_mat(offset)); }
        Mesh& Scale(const Math::fVector3& scale) { return ApplyTransform(Math::Matrix3D::scale_mat(scale)); }
        Mesh& Rotate(const Math::fVector3& rot)  { return ApplyTransform(Math::Matrix3D::rotate_mat(rot)); }

        void AddTo(RenderData& rd) const;

        void PushVertex(const Vertex& v) { vertices.push_back(v); }
        void PushIndex(TriIndices i) { indices.push_back(i); }

        struct BatchProxy {
            u32 iOffset;
            Mesh& mesh;

            void PushV(const Vertex& v) { mesh.PushVertex(v); }
            void ReserveV(u32 amount) { mesh.vertices.resize(mesh.vertices.size() + amount); }
            Vertex& VertAt(u32 i) { return mesh.vertices[iOffset + i]; }

            void PushI(TriIndices i) { mesh.PushIndex(i + iOffset); }
            void PushI(u32 i, u32 j, u32 k) { mesh.PushIndex({ i + iOffset, j + iOffset, k + iOffset }); }
            void ReserveI(u32 amount) { mesh.indices.resize(mesh.indices.size() + amount); }
        };

        BatchProxy NewBatch() { return { (u32)vertices.size(), *this }; }

        template <Fn<T, Math::fVector3> F> void AddQuad(const Primitives::Quad& quad, F&& f);
        template <Fn<T, Math::fVector3> F> void AddTri(const Primitives::Tri& tri, F&& f);

        // moving isnt that efficient bc vertexes are easy to copy
        Mesh& Add(const Mesh& m);
        static Mesh Combine(IList<Mesh> meshes);

        template <class U> Mesh& ApplyMaterial(U Vertex::* prop, U base);
        template <class U> Mesh& ApplyMaterial(isize prop, U base);

        template <class U, Fn<U, T> F> Mesh<U> Convert(F&& f) const;

        void Clear();

        friend class GraphicsDevice;
        friend class RenderData;
        friend class GenericMesh;

        friend class Primitives::Tri;
        friend class Primitives::Quad;
    };

    template <class T>
    void Mesh<T>::AddTo(RenderData& rd) const {
        rd.PushIndicesOffseted(indices, sizeof(Vertex));

        const Math::Matrix3D normMat = modelTransform.inv().transpose();
        for (const Vertex& v : vertices) {
            rd.PushVertex(VertexMul(v, modelTransform, normMat));
        }
    }

    template <class T>
    Mesh<T>& Mesh<T>::Add(const Mesh& m) {
        const usize off = vertices.size();
        vertices.insert(vertices.end(), m.vertices.begin(), m.vertices.end());
        const usize end = indices.size();
        indices.insert(indices.end(), m.indices.begin(), m.indices.end());
        std::for_each(indices.begin() + end, indices.end(), [=](TriIndices& i) { i += off; });
        return *this;
    }

    template <class T>
    Mesh<T>& Mesh<T>::ApplyTransform(const Math::Matrix3D& model) {
        const Math::Matrix3D normMat = model.inv().transpose();
        for (auto& v : vertices) v = VertexMul(v, model, normMat);
        return *this;
    }

    template <class T> Mesh<T> Mesh<T>::Combine(IList<Mesh> meshes) {
        Mesh combined = std::move(meshes.begin()[0]);
        for (usize i = 0; i < meshes.size(); ++i)
            combined.Add(meshes.begin()[i]);
        return combined;
    }

    template <class T> template <class U>
    Mesh<T>& Mesh<T>::ApplyMaterial(U Vertex::* prop, U base) {
        for (auto& v : vertices) {
            v.*prop = base;
        }
        return *this;
    }

    template<class T> template<class U>
    Mesh<T>& Mesh<T>::ApplyMaterial(const isize prop, U base) {
        void* beg = ((void*)vertices[0]) + prop;
        for (int i = 0; i < vertices.size(); ++i, beg += sizeof(T)) {
            *((U*)beg) = base;
        }
        return *this;
    }

    template <class T>
    template <class U, Fn<U, T> F> Mesh<U> Mesh<T>::Convert(F&& f) const {
        Vec<U> newVerts {};
        newVerts.resize(vertices.size());
        std::transform(vertices.begin(), vertices.end(), newVerts.begin(), std::forward<F>(f));
        return Mesh<U>(newVerts, indices);
    }

    template<class T>
    void Mesh<T>::Clear() {
        vertices.clear();
        indices.clear();
    }
}