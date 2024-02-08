#pragma once
#include <vector>
#include <algorithm>

#include "RenderObject.h"
#include "VertexElement.h"
#include "TriIndices.h"

namespace Graphics {
    class GenericMesh;
    
    template <class T>
    class Mesh {
    public:
        using Vertex = T;
    private:
        std::vector<Vertex> vertices;
        std::vector<TriIndices> indices;

        Maths::mat3D modelTransform;

        RenderData* render = nullptr;
        uint deviceIndex = 0;
    public:
        Mesh() = default;
        Mesh(const std::vector<Vertex>& v, const std::vector<TriIndices>& i)
            : vertices(v), indices(i) {}
        Mesh(std::vector<Vertex>&& v, std::vector<TriIndices>&& i)
            : vertices(std::move(v)), indices(std::move(i)) {}

        ~Mesh() { Unbind(); }

        static void Clone(Mesh& dest, const Mesh& from);
        Mesh(const Mesh& mesh) { Clone(*this, mesh); }
        Mesh& operator=(const Mesh& mesh) { Clone(*this, mesh); return *this; } // NOLINT(bugprone-unhandled-self-assignment) idk why it thinks this is bad

        static void Transfer(Mesh& dest, Mesh&& from);
        Mesh(Mesh&& mesh) noexcept { Transfer(*this, std::move(mesh)); }
        Mesh& operator=(Mesh&& mesh) noexcept { Transfer(*this, std::move(mesh)); return *this; }
        Mesh& Replace(Mesh&& mesh);

        void SetTransform(const Maths::mat3D& model) { modelTransform = model; }
        void Transform(const Maths::mat3D& model) { modelTransform *= model; }
        void ApplyTransform(const Maths::mat3D& model);
        void AddTo(DynamicVertexBuffer& vbuffer, DynamicIndexBuffer& ibuffer) const;

        // moving isnt that efficient bc vertexes are easy to copy
        Mesh& Add(const Mesh& m);
        template <size_t N> static Mesh Combine(std::array<Mesh, N> meshes);

        template <class U> Mesh& ApplyMaterial(U Vertex::* prop, U base);
        template <class U> Mesh& ApplyMaterial(std::ptrdiff_t prop, U base);

        template <class U, class F> Mesh<U> Convert(F f);

        [[nodiscard]] bool IsBound() const { return render; }

        void Bind(RenderData& rend);
        void Bind(RenderObject<T>& rend);
        void Unbind();

        std::vector<Vertex>& GetVertices() { return vertices; }
        [[nodiscard]] const std::vector<Vertex>& GetVertices() const { return vertices; }
        std::vector<TriIndices>& GetIndices() { return indices; }
        [[nodiscard]] const std::vector<TriIndices>& GetIndices() const { return indices; }

        friend class GraphicsDevice;
        friend class RenderData;
        friend class GenericMesh;
    };

    template <class T>
    void Mesh<T>::AddTo(DynamicVertexBuffer& vbuffer, DynamicIndexBuffer& ibuffer) const {
        std::vector<T> transformed;
        transformed.resize(vertices.size());
        std::transform(vertices.begin(), vertices.end(), transformed.begin(), [&](auto v) { return GL_VERTEX_MATMUL(v, modelTransform); });
        vbuffer.AddData(transformed);
        ibuffer.AddData((const uint*)indices.data(), indices.size() * 3, vertices.size() - 1);
    }

    template <class T>
    Mesh<T>& Mesh<T>::Add(const Mesh& m) {
        uint off = vertices.size();
        vertices.insert(vertices.end(), m.vertices.begin(), m.vertices.end());
        auto end = indices.size();
        indices.insert(indices.end(), m.indices.begin(), m.indices.end());
        std::for_each(indices.begin() + end, indices.end(), [=](TriIndices& i) { i += off; });
        return *this;
    }

    template<class T>
    void Mesh<T>::Clone(Mesh& dest, const Mesh& from) {
        dest.vertices = from.vertices;
        dest.indices = from.indices;
        dest.modelTransform = from.modelTransform;
    }

    template<class T>
    void Mesh<T>::Transfer(Mesh& dest, Mesh&& from) {
        dest.vertices = std::move(from.vertices);
        dest.indices = std::move(from.indices);

        dest.modelTransform = from.modelTransform;

        if (dest.render) dest.render->UnbindMesh(dest.deviceIndex);
        dest.render = from.render;
        from.render = nullptr;

        dest.deviceIndex = from.deviceIndex;
        from.deviceIndex = 0;
        
        if (dest.render)
            dest.render->GetMeshes()[dest.deviceIndex].Set(&dest);
    }

    template <class T> Mesh<T>& Mesh<T>::Replace(Mesh&& mesh) {
        vertices = std::move(mesh.vertices);
        indices = std::move(mesh.indices);
        modelTransform = mesh.modelTransform;
        return *this;
    }

    template <class T>
    void Mesh<T>::ApplyTransform(const Maths::mat3D& model) {
        for (auto& v : vertices) v *= model;
    }

    template <class T>
    void Mesh<T>::Bind(RenderData& rend) { 
        deviceIndex = rend.GetMeshes().size();
        rend.GetMeshes().push_back(this);
        this->render = &rend;
    }

    template <class T>
    void Mesh<T>::Bind(RenderObject<T>& rend) { 
        Bind(rend.GetRenderData());
    }

    template <class T>
    void Mesh<T>::Unbind() {
        if (render)
            render->UnbindMesh((int)deviceIndex);
        render = nullptr;
    }

    template <class T>
    template <size_t N>
    Mesh<T> Mesh<T>::Combine(std::array<Mesh, N> meshes) {
        if constexpr (N == 0) return Mesh<T>();
        Mesh<T> combined = std::move(meshes[0]);
        for (size_t i = 0; i < N; ++i)
            combined.Add(meshes[i]);
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
    Mesh<T>& Mesh<T>::ApplyMaterial(std::ptrdiff_t prop, U base) {
        void* beg = ((void*)vertices[0]) + prop;
        for (int i = 0; i < vertices.size(); ++i, beg += sizeof(T)) {
            *((U*)beg) = base;
        }
        return *this;
    }

    template <class T> template <class U, class F> Mesh<U> Mesh<T>::Convert(F f) {
        std::vector<U> newVerts {};
        newVerts.resize(vertices.size());
        std::transform(vertices.begin(), vertices.end(), newVerts.begin(), f);
        return Mesh<U>(newVerts, indices);
    }

    namespace MeshUtils {
        using Vec3 = const Maths::fvec3&;
        OPENGL_API Mesh<VertexColor3D> CubeMesh(Vec3 origin, float x, float y, float z);
        OPENGL_API Mesh<Maths::fvec2> CircleMesh(float radius, int subdivisions);
        // TODO: MeshObject CubeMesh(Vec3 origin, Vec3 x, Vec3 y, Vec3 z);
    }
}
