#pragma once
#include "RenderData.h"

#include "stdu/ref.h"

namespace Graphics {
    template <class T>
    class RenderObject {
        stdu::ref<RenderData> rd;
    public:
        RenderObject() {}
        RenderObject(RenderData& d) : rd(d) {}

        RenderObject(const RenderObject& ro) : rd(ro.rd) {}
		RenderObject& operator=(const RenderObject& ro) { rd = ro.rd; return *this; }
        
		RenderObject(RenderObject&& ro) noexcept : rd(ro.rd) { ro.rd = nullptr; }
		RenderObject& operator=(RenderObject&& ro) noexcept { rd = ro.rd; ro.rd = nullptr; return *this; }

        ~RenderObject() { }
           
		void Bind() const { rd->Bind(); }
		void Unbind() const { rd->Unbind(); }

		void SetVert(const T* data, uint count)        { rd->SetVert<T>(data, count); }
		template <class U> void SetVert(const U& arr)  { rd->SetVert<U>(arr); }
		void AddVert(const T* data, uint count)        { rd->AddVert<T>(data, count); }
		template <class U> void AddVert(const U& arr)  { rd->AddVert<U>(arr); }
		void ClearVert(bool shallowClear = true) { rd->ClearData(shallowClear); }
		[[nodiscard]] uint GetVertLength() const { return rd->GetVertLength(); }
	    
		void SetInd(const T* data, uint count)       { rd->SetInd<T>(data, count); }
		template <class U> void SetInd(const U& arr) { rd->SetInd<U>(arr); }
		void AddInd(const T* data, uint count)       { rd->AddInd<T>(data, count); }
		template <class U> void AddInd(const U& arr) { rd->AddInd<U>(arr); }
		void ClearInd(bool shallowClear = true) { rd->ClearInd(shallowClear); }
		[[nodiscard]] uint GetIndLength() const { return rd->GetIndLength(); }

                            DynamicVertexBuffer& VertObj()           { return rd->VertObj(); }
        [[nodiscard]] const DynamicVertexBuffer& VertObj()     const { return rd->VertObj(); }
		                    DynamicIndexBuffer&  IndObj()            { return rd->IndObj(); }
		[[nodiscard]] const DynamicIndexBuffer&  IndObj()      const { return rd->IndObj(); }
		                    VertexArray&         VertArr()           { return rd->VertArr(); }
		[[nodiscard]] const VertexArray&         VertArr()     const { return rd->VertArr(); }
		                    class GraphicsDevice& GraphicsDevice()       { return rd->GraphicsDevice(); }
		[[nodiscard]] const class GraphicsDevice& GraphicsDevice() const { return rd->GraphicsDevice(); }
		                    std::vector<GenericMesh>& Meshes()       { return rd->Meshes(); }
		[[nodiscard]] const std::vector<GenericMesh>& Meshes() const { return rd->Meshes(); }
	                        class Shader& Shader()                   { return rd->Shader(); }
	    [[nodiscard]] const class Shader& Shader()             const { return rd->Shader(); }
    	                    Maths::mat3D& ViewMat()                  { return rd->ViewMat(); }
    	[[nodiscard]] const Maths::mat3D& ViewMat()            const { return rd->ViewMat(); }
    	                    Maths::mat3D& ProjectionMat()            { return rd->ProjectionMat(); }
    	[[nodiscard]] const Maths::mat3D& ProjectionMat()      const { return rd->ProjectionMat(); }
        
                            RenderData& GetRenderData()       { return *rd; }
        [[nodiscard]] const RenderData& GetRenderData() const { return *rd; }

		void ClearData(bool shallowClear = true) { rd->ClearData(shallowClear); }

		void BindMeshes(Mesh<T>& mesh) { rd->BindMeshes<T>(std::span { &mesh, 1 }); }
		void BindMeshes(std::span<Mesh<T>> meshes) { rd->BindMeshes<T>(meshes); }
        void BindMeshes(std::initializer_list<Mesh<T>*> meshes) { for (auto* m : meshes) BindMeshes(*m); }
		template <stdu::array_like U> void BindMeshes(U& ms) { BindMeshes(stdu::to_span(ms)); }
        
		void Render(class Shader& rShader, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->Render(rShader, args, setDefaultShaderArgs); }
		void Render(const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->Render(args, setDefaultShaderArgs); }

    	void RenderInstanced(class Shader& rShader, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->RenderInstanced(rShader, instances, args, setDefaultShaderArgs); }
    	void RenderInstanced(int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->RenderInstanced(instances, args, setDefaultShaderArgs); }
        
		void AddNewMeshes(const Mesh<T>& mesh) { rd->AddNewMeshes<T>(std::span { &mesh, 1 }); }
		void AddNewMeshes(std::span<const Mesh<T>> meshes) { rd->AddNewMeshes<T>(meshes); }
		template <stdu::array_like U> void AddNewMeshes(const U& arr) { AddNewMeshes(stdu::to_cspan(arr)); }
		void AddBoundMeshes() { rd->AddBoundMeshes<T>(); }
        
		void ResetData(bool shallowClear = true) { rd->ResetData<T>(shallowClear); }
        
		void UnbindMesh(int index) { rd->UnbindMesh(index); }
		void UnbindMeshes(int indexStart, int indexEnd) { rd->UnbindMeshes(indexStart, indexEnd); }
		void UpdateMeshIndices() { rd->UpdateMeshIndices(); }
        
		void Destroy() { rd->Destroy(); }

	    void SetCamera(const Maths::mat3D& cam) { rd->SetCamera(cam); }
	    void SetProjection(const Maths::mat3D& proj) { rd->SetProjection(proj); }
	    
	    void UseShader(std::string_view code) { rd->UseShader(code); }
	    void UseShaderFromFile(const std::string& file) { rd->UseShaderFromFile(file); }
	    void UseShaderFromFile(const std::string& vert, const std::string& frag, const std::string& geom = {})
    	{ rd->UseShaderFromFile(vert, frag, geom); }
	    void EnableShader()  { rd->EnableShader();  }
	    void DisableShader() { rd->DisableShader(); }
    };
}
