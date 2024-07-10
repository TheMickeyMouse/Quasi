#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GenericMesh.h"
#include "GLDebug.h"
#include "Shader.h"
#include "VertexElement.h"

#include <cstring>

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
	private:
		UniqueRef<byte[]> vertexData;
		u32 vertexOffset = 0;
		UniqueRef<u32[]> indexData;
		u32 indexOffset = 0;

		Ref<GraphicsDevice> device = nullptr;
		u32 deviceIndex = 0;

		friend class GraphicsDevice;
	public:
		explicit RenderData() = default;
		explicit RenderData(u32 vsize, u32 isize, u32 vertSize, VertexDebugTypeIndex vertType, const VertexBufferLayout& layout) :
			vbo(vsize * vertSize), ibo(isize), typeindex(vertType),
			vertexData(new byte[vertSize * vsize]), indexData(new u32[isize]) {
			varray.Create();
			varray.Bind();
			varray.AddBuffer(layout);
		}

		template <class T> static void Create(u32 vsize, u32 isize, RenderData& out);

		RenderData(const RenderData&) = delete;
		RenderData& operator=(const RenderData&) = delete;

		static void Transfer(RenderData& dest, RenderData&& from);
		RenderData(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); }
		RenderData& operator=(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); return *this; }

		~RenderData();

		template <class T> void PushVertex(const T& vertex);
		void PushIndex(TriIndices index);
		void PushIndicesOffseted(Span<const TriIndices> indices, usize objectSize);

		void Bind() const;
		void Unbind() const;

		[[nodiscard]] VertexDebugTypeIndex GetType() const { return typeindex; }

		void BufferUnload();
		void BufferLoad();

		void Clear();
		template <class T> void Add(const Mesh<T>& mesh) { mesh.AddTo(*this); }
		template <CollectionLike T> void Add(const T& arr) { for (const auto& m : arr) Add(m); }

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
		template <class T> friend class Mesh;
	};

	template <class T>
	void RenderData::Create(u32 vsize, u32 isize, RenderData& out) {
		// times 3 to account for triangles
		out = RenderData(vsize, isize * 3, sizeof(T), VertexDebugTypeInfo::of<T>(), VertexLayoutOf<T>());
	}

	template <class T> void RenderData::PushVertex(const T& vertex) {
		const byte* rawbytes = reinterpret_cast<const byte*>(&vertex);
		std::memcpy(vertexData.get() + vertexOffset, rawbytes, sizeof(T));
		vertexOffset += sizeof(T);
	}
}
