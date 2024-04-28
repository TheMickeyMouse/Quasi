#pragma once
#include "VertexArray.h"
#include "DynamicVertexBuffer.h"
#include "DynamicIndexBuffer.h"
#include "GenericMesh.h"
#include "Shader.h"
#include "VertexElement.h"

namespace Graphics {
    template <class>
	class RenderObject;
    
	class RenderData {
		VertexArray varray { {} };
		DynamicVertexBuffer vbo;
		DynamicIndexBuffer ibo;

		std::vector<GenericMesh> meshes {};

	    Maths::mat3D projection = Maths::mat3D::ortho_projection({ -4, 4, -3, 3, 0.1f, 100 });
	    Maths::mat3D camera {};
	    Shader shader = {}; // shader can be null if renderId is 0

		class GraphicsDevice* device = nullptr;
		uint deviceIndex = 0;
		friend GraphicsDevice;
	public:
		explicit RenderData(uint vsize, uint isize, uint vertSize,
				   VertexDebugTypeIndex vertType, const VertexBufferLayout& layout) :
			vbo(vsize, vertSize, vertType), ibo(isize) {
			varray.Bind();
			varray.AddBuffer(layout);
		}

		RenderData(const RenderData&) = delete;
		RenderData& operator=(const RenderData&) = delete;

		static void Transfer(RenderData& dest, RenderData&& from);
		RenderData(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); }
		RenderData& operator=(RenderData&& rd) noexcept { Transfer(*this, std::move(rd)); return *this; }

		~RenderData();

		template <class T> static RenderData Create(uint vsize, uint isize);
	    template <class T> static std::unique_ptr<RenderData> CreateHeap(uint vsize, uint isize);

		void Bind() const;
		void Unbind() const;

		[[nodiscard]] VertexDebugTypeIndex GetType() const { return vbo.GetType(); }

		template <class T> void SetVert(std::span<const T> data) { vbo.SetData(data); }
		template <stdu::array_like T> void SetVert(const T& arr) { vbo.SetData(arr); }
		template <class T> void AddVert(std::span<const T> data) { vbo.AddData(data); }
		template <stdu::array_like T> void AddVert(const T& arr) { vbo.AddData(arr); }
		void ClearVert(bool shallowClear = true) { vbo.ClearData(shallowClear); }
		[[nodiscard]] uint GetVertLength() const { return vbo.GetLength(); }
	    
		void SetInd(std::span<const uint> data) { ibo.SetData(data); }
		template <stdu::array_like T> void SetInd(const T& arr) { ibo.SetData(arr); }
		void AddInd(std::span<const uint> data) { ibo.AddData(data); }
		template <stdu::array_like T> void AddInd(const T& arr) { ibo.AddData(arr); }
		void ClearInd(bool shallowClear = true) { ibo.ClearData(shallowClear); }
		[[nodiscard]] uint GetIndLength() const { return ibo.GetLength(); }

		DynamicVertexBuffer& VertObj() { return vbo; }
		[[nodiscard]] const DynamicVertexBuffer& VertObj() const { return vbo; }
		DynamicIndexBuffer& IndObj() { return ibo; }
		[[nodiscard]] const DynamicIndexBuffer& IndObj() const { return ibo; }
		VertexArray& VertArr() { return varray; }
		[[nodiscard]] const VertexArray& VertArr() const { return varray; }
		class GraphicsDevice& GraphicsDevice() { return *device; }
		[[nodiscard]] const class GraphicsDevice& GraphicsDevice() const { return *device; }
		std::vector<GenericMesh>& Meshes() { return meshes; }
		[[nodiscard]] const std::vector<GenericMesh>& Meshes() const { return meshes; }
		Maths::mat3D& ViewMat() { return camera; }
		[[nodiscard]] const Maths::mat3D& ViewMat() const { return camera; }
		Maths::mat3D& ProjectionMat() { return projection; }
		[[nodiscard]] const Maths::mat3D& ProjectionMat() const { return projection; }

		void ClearData(bool shallowClear = true);
		template <class T> void BindMeshes(std::span<Mesh<T>> newMeshes);
		template <stdu::array_like T> void BindMeshes(T& ms) { BindMeshes(stdu::to_span(ms)); }

		template <class T> void AddNewMeshes(std::span<const Mesh<T>> newMeshes);
		template <stdu::array_like T> void AddNewMeshes(const T& arr) { AddNewMeshes(stdu::to_cspan(arr)); }
		template <class T> void AddBoundMeshes() { for (GenericMesh& m : meshes) m.As<T>().AddTo(vbo, ibo); }
		template <class T> void ResetData(bool shallowClear = true) { ClearData(shallowClear); AddBoundMeshes<T>(); }

		void UnbindMesh(int index);
		void UnbindMeshes(int indexStart, int indexEnd);

		void UpdateMeshIndices();

		void Destroy();

		void Render(Shader& replaceShader, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
		void Render(const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { Render(shader, args, setDefaultShaderArgs); }

		void RenderInstanced(Shader& replaceShader, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true);
		void RenderInstanced(int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { RenderInstanced(shader, instances, args, setDefaultShaderArgs); }

	    void SetCamera(const Maths::mat3D& cam) { camera = cam; }
	    void SetProjection(const Maths::mat3D& proj) { projection = proj; }

	    class Shader& Shader() { return shader; }
	    [[nodiscard]] const class Shader& Shader() const { return shader; }
	    
	    void UseShader(std::string_view code) { shader = Graphics::Shader { code }; }
	    void UseShaderFromFile(const std::string& file) { shader = Shader::FromFile(file); }
	    void UseShaderFromFile(const std::string& vert, const std::string& frag, const std::string& geom = {})
		{ shader = Shader::FromFile(vert, frag, geom); }
	    void EnableShader();
	    void DisableShader();

		friend class GraphicsDevice;
	};

	template <class T>
	RenderData RenderData::Create(uint vsize, uint isize) {
		// times 3 to account for triangles
		return RenderData(vsize, isize * 3, sizeof(T), VertexDebugTypeInfo::of<T>(), VertexLayoutOf<T>());
	}

    template <class T>
    std::unique_ptr<RenderData> RenderData::CreateHeap(uint vsize, uint isize) {
	    // times 3 to account for triangles
	    return std::make_unique<RenderData>(
			vsize, isize * 3, (uint)sizeof(T), VertexDebugTypeInfo::of<T>(), VertexLayoutOf<T>());
	}

	template <class T>
	void RenderData::BindMeshes(std::span<Mesh<T>> newMeshes) {
		for (Mesh<T>& m : newMeshes) {
			m.Bind(*this);
		}
	}

	template<class T>
	void RenderData::AddNewMeshes(std::span<const Mesh<T>> newMeshes) {
		for (const Mesh<T>& m : newMeshes)
			m.AddTo(vbo, ibo);
	}
}