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

                            DynamicVertexBuffer& GetVertObj()           { return rd->GetVertObj(); }
        [[nodiscard]] const DynamicVertexBuffer& GetVertObj()     const { return rd->GetVertObj(); }
		                    DynamicIndexBuffer&  GetIndObj()            { return rd->GetIndObj(); }
		[[nodiscard]] const DynamicIndexBuffer&  GetIndObj()      const { return rd->GetIndObj(); }
		                    VertexArray&         GetVertArr()           { return rd->GetVertArr(); }
		[[nodiscard]] const VertexArray&         GetVertArr()     const { return rd->GetVertArr(); }
		                    GraphicsDevice& GetGraphicsDevice()         { return rd->GetGraphicsDevice(); }
		[[nodiscard]] const GraphicsDevice& GetGraphicsDevice()   const { return rd->GetGraphicsDevice(); }
		                    std::vector<GenericMesh>& GetMeshes()       { return rd->GetMeshes(); }
		[[nodiscard]] const std::vector<GenericMesh>& GetMeshes() const { return rd->GetMeshes(); }
	                        Shader& GetShader()                         { return rd->GetShader(); }
	    [[nodiscard]] const Shader& GetShader()                   const { return rd->GetShader(); }
        
                            RenderData& GetRenderData()       { return *rd; }
        [[nodiscard]] const RenderData& GetRenderData() const { return *rd; }

		void ClearData(bool shallowClear = true) { rd->ClearData(shallowClear); }

		void BindMeshes(Mesh<T>* meshes, uint count) { rd->BindMeshes<T>(meshes, count); }
        void BindMeshes(std::initializer_list<Mesh<T>*> meshes) { for (auto* m : meshes) BindMeshes(m, 1); }
		template <class U> void BindMeshes(U& ms) { BindMeshes(ms.begin(), (uint)ms.size()); }
        
		void Render() { rd->Render(); }
        
		void AddNewMeshes(const Mesh<T>* meshes, uint count) { rd->AddNewMeshes<T>(meshes, count); }
		template <class U> void AddNewMeshes(const U& arr) { AddNewMeshes(arr.begin(), (uint)arr.size()); }
		void AddBoundMeshes() { rd->AddBoundMeshes<T>(); }
        
		void ResetData(bool shallowClear = true) { rd->ResetData<T>(shallowClear); }
        
		void UnbindMesh(int index) { rd->UnbindMesh(index); }
		void UnbindMeshes(int indexStart, int indexEnd) { rd->UnbindMeshes(indexStart, indexEnd); }
		void UpdateMeshIndices() { rd->UpdateMeshIndices(); }
        
		void Destroy() { rd->Destroy(); }

	    void SetCamera(const Maths::mat3D& cam) { rd->SetCamera(cam); }
	    void SetProjection(const Maths::mat3D& proj) { rd->SetProjection(proj); }
	    
	    void UseShader(const std::string& code) { rd->UseShader(code); }
	    void UseShaderFromFile(const std::string& file) { rd->UseShaderFromFile(file); }
	    void EnableShader()  { rd->EnableShader();  }
	    void DisableShader() { rd->DisableShader(); }
    };
}
