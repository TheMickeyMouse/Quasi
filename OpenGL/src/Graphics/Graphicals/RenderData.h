#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GenericMesh.h"
#include "GLDebug.h"
#include "Shader.h"
#include "VertexElement.h"

namespace Quasi::Graphics {
	class GraphicsDevice;

    template <class>
	class RenderObject;
    
	class RenderData {
	public:
		VertexArray varray;
		VertexBuffer vbo;
		IndexBuffer ibo;

		VertexDebugTypeIndex typeindex;

	    Math::Matrix3D projection = Math::Matrix3D::ortho_projection({ -4, 4, -3, 3, 0.1f, 100 });
	    Math::Matrix3D camera {};
	    Shader shader = {}; // shader can be null if renderId is 0

		Vec<GenericMesh> meshes {};
	private:
		Ref<GraphicsDevice> device = nullptr;
		u32 deviceIndex = 0;

		friend class GraphicsDevice;
	public:
		explicit RenderData() = default;
		explicit RenderData(u32 vsize, u32 isize, u32 vertSize,
				   VertexDebugTypeIndex vertType, const VertexBufferLayout& layout) :
			vbo(vsize, vertSize), ibo(isize), typeindex(vertType) {
			varray.Create();
			varray.Bind();
			varray.AddBuffer(layout);
		}

		RenderData(const RenderData&) = delete;
		RenderData& operator=(const RenderData&) = delete;

		static void Transfer(RenderData& dest, RenderData&& from);
		RenderData(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); }
		RenderData& operator=(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); return *this; }

		~RenderData();

		template <class T> static void Create(u32 vsize, u32 isize, RenderData& out);

		void Bind() const;
		void Unbind() const;

		[[nodiscard]] VertexDebugTypeIndex GetType() const { return typeindex; }

		void ClearData();
		template <class T>     void BindMeshes(Span<Mesh<T>> newMeshes);
		template <ArrayLike T> void BindMeshes(T& ms) { BindMeshes(TakeSpan(ms)); }

		template <class T>     void AddNewMeshes(Span<const Mesh<T>> newMeshes);
		template <ArrayLike T> void AddNewMeshes(const T& arr) { AddNewMeshes(TakeSpan(arr)); }
		template <class T> void AddBoundMeshes() { for (GenericMesh& m : meshes) m.As<T>().AddTo(vbo, ibo); }
		template <class T> void ResetData() { ClearData(); AddBoundMeshes<T>(); }

		void UnbindMesh(u32 index);
		void UnbindMeshes(u32 indexStart, u32 indexEnd);

		void UpdateMeshIndices();

		void Destroy();

		void Render(Shader& replaceShader, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
		void Render(const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(shader, args, setDefaultShaderArgs); }

		void RenderInstanced(Shader& replaceShader, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
		void RenderInstanced(int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { RenderInstanced(shader, instances, args, setDefaultShaderArgs); }

	    void SetCamera(const Math::Matrix3D& cam) { camera = cam; }
	    void SetProjection(const Math::Matrix3D& proj) { projection = proj; }
	    
	    void UseShader(Str code) { shader = Shader { code }; }
	    void UseShaderFromFile(Str file) { shader = Shader::FromFile(file); }
	    void UseShaderFromFile(Str vert, Str frag, Str geom = {}) { shader = Shader::FromFile(vert, frag, geom); }

		friend class GraphicsDevice;
	};

	template <class T>
	void RenderData::Create(u32 vsize, u32 isize, RenderData& out) {
		// times 3 to account for triangles
		out = RenderData(vsize, isize * 3, sizeof(T), VertexDebugTypeInfo::of<T>(), VertexLayoutOf<T>());
	}

	template <class T>
	void RenderData::BindMeshes(Span<Mesh<T>> newMeshes) {
		for (Mesh<T>& m : newMeshes) {
			m.Bind(*this);
		}
	}

	template<class T>
	void RenderData::AddNewMeshes(Span<const Mesh<T>> newMeshes) {
		for (const Mesh<T>& m : newMeshes)
			m.AddTo(vbo, ibo);
	}
}