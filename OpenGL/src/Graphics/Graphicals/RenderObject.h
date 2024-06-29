#pragma once
#include "RenderData.h"

#include "Utils/Ref.h"

namespace Quasi::Graphics {
    template <class T>
    class RenderObject {
        Ref<RenderData> rd;
    public:
        RenderObject() = default;
        RenderObject(RenderData& d) : rd(d) {}

        RenderObject(const RenderObject& ro) : rd(ro.rd) {}
		RenderObject& operator=(const RenderObject& ro) { rd = ro.rd; return *this; }
        
		RenderObject(RenderObject&& ro) noexcept : rd(ro.rd) { ro.rd = nullptr; }
		RenderObject& operator=(RenderObject&& ro) noexcept { rd = ro.rd; ro.rd = nullptr; return *this; }

        ~RenderObject() { }
           
		void Bind() const { rd->Bind(); }
		void Unbind() const { rd->Unbind(); }

                            RenderData& GetRenderData()       { return *rd; }
        [[nodiscard]] const RenderData& GetRenderData() const { return *rd; }

        [[nodiscard]] const RenderData* operator->() const { return &GetRenderData(); }
    	RenderData* operator->() { return &GetRenderData(); }

		void ClearData() { rd->ClearData(); }

		void BindMesh(Mesh<T>& mesh) { rd->BindMeshes<T>(Span { &mesh, 1 }); }
		void BindMeshes(Span<Mesh<T>> meshes) { rd->BindMeshes<T>(meshes); }
        void BindMeshes(IList<Mesh<T>*> meshes) { for (auto* m : meshes) BindMesh(*m); }
        
		void Render(Shader& rShader, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->Render(rShader, args, setDefaultShaderArgs); }
		void Render(const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->Render(args, setDefaultShaderArgs); }

    	void RenderInstanced(Shader& rShader, int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->RenderInstanced(rShader, instances, args, setDefaultShaderArgs); }
    	void RenderInstanced(int instances, const ShaderArgs& args = {}, bool setDefaultShaderArgs = true) { rd->RenderInstanced(instances, args, setDefaultShaderArgs); }
        
		void AddNewMesh(const Mesh<T>& mesh) { rd->AddNewMeshes<T>(Span { &mesh, 1 }); }
		void AddNewMeshes(Span<const Mesh<T>> meshes) { rd->AddNewMeshes<T>(meshes); }
		void AddBoundMeshes() { rd->AddBoundMeshes<T>(); }
        
		void ResetData() { rd->ResetData<T>(); }
        
		void UnbindMesh(u32 index) { rd->UnbindMesh(index); }
		void UnbindMeshes(u32 indexStart, u32 indexEnd) { rd->UnbindMeshes(indexStart, indexEnd); }
		void UpdateMeshIndices() { rd->UpdateMeshIndices(); }
        
		void Destroy() { rd->Destroy(); rd = nullptr; }

	    void SetCamera(const Math::Matrix3D& cam) { rd->SetCamera(cam); }
	    void SetProjection(const Math::Matrix3D& proj) { rd->SetProjection(proj); }
	    
	    void UseShader(Str code) { rd->UseShader(code); }
	    void UseShaderFromFile(Str file) { rd->UseShaderFromFile(file); }
	    void UseShaderFromFile(Str vert, Str frag, Str geom = {})
    	{ rd->UseShaderFromFile(vert, frag, geom); }
    };
}
