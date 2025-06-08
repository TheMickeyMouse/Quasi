#pragma once
#include "RenderData.h"

#include "Utils/Ref.h"

namespace Quasi::Graphics {
	struct DrawOptions {
		OptRef<Shader> shader;
		ShaderArgs arguments = {};
		bool useDefaultArguments = true;
	};

	inline DrawOptions UseShader(Shader& shader, bool useDefaultArgs = true) {
		return { .shader = shader, .useDefaultArguments = useDefaultArgs };
	}
	inline DrawOptions UseShaderWithArgs(Shader& shader, ShaderArgs arguments, bool useDefaultArgs = true) {
		return { .shader = shader, .arguments = std::move(arguments), .useDefaultArguments = useDefaultArgs };
	}
	inline DrawOptions UseArgs(ShaderArgs arguments, bool useDefaultArgs = true) {
		return { .shader = nullptr, .arguments = std::move(arguments), .useDefaultArguments = useDefaultArgs };
	}

    template <class T>
    class RenderObject {
        OptRef<RenderData> rd;
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
        const RenderData& GetRenderData() const { return *rd; }

        const RenderData* operator->() const { return &GetRenderData(); }
    	RenderData* operator->() { return &GetRenderData(); }

		void Draw(IList<const Mesh<T>*> meshes, const DrawOptions& options = {});
		void Draw(const Mesh<T>& mesh, const DrawOptions& options = {}) { Draw({ &mesh }, options); }
		void Draw(const CollectionAny auto& meshes, const DrawOptions& options = {});
		void DrawInstanced(IList<const Mesh<T>*> meshes, int instances, const DrawOptions& options = {});
    	void DrawInstanced(const Mesh<T>& mesh, int instances, const DrawOptions& options = {}) { DrawInstanced({ &mesh }, instances, options); }
    	void DrawInstanced(const CollectionAny auto& meshes, int instances, const DrawOptions& options = {});

    	void BeginContext() { rd->BufferUnload(); rd->Clear(); }
    	void AddMesh(const Mesh<T>& mesh) { rd->Add(mesh); }
    	void AddMeshes(const CollectionAny auto& meshes) { for (const Mesh<T>& m : meshes) rd->Add(m); }
    	void AddMeshes(IList<const Mesh<T>*> meshes) { for (auto* m : meshes) rd->Add(*m); }

		struct RawBatch {
    		u32 iOffset;
    		Ref<RenderData> rd;

    		void PushV(const T& v) { rd->PushVertex(v); }
    		void ResizeV(u32) const {}
    		void ReserveV(u32) const {}
    		T& VertAt(u32 i) { return rd->vertexData->Transmute<T>()[i]; }
    		const T& VertAt(u32 i) const { return rd->vertexData->Transmute<T>()[i]; }
    		u32 VertCount() const { return rd->vertexOffset / sizeof(T) - iOffset; }

    		void ResizeI(u32) const {}
    		void ReserveI(u32) const {}
    		void PushI(u32 i, u32 j, u32 k) { rd->PushIndex({ i + iOffset, j + iOffset, k + iOffset }); }
    		TriIndices* IndexData() { return Memory::TransmutePtr<TriIndices>(rd->indexData.Data()) + iOffset; }
    	};

		void AddMeshB(auto&& meshBuilder, auto&& gpass) {
    		meshBuilder.Merge((decltype(gpass))gpass, RawBatch { rd->vertexOffset / sizeof(T), rd });
    	}

    	void EndContext() { rd->BufferLoad(); }

     	void DrawContext(const DrawOptions& options = {}) {
    		rd->Render(Memory::AsMut(options.shader.UnwrapOr(rd->shader)), options.arguments, options.useDefaultArguments);
    	}
    	void DrawContextInstanced(int instances, const DrawOptions& options = {}) {
    		rd->RenderInstanced(Memory::AsMut(options.shader.UnwrapOr(rd->shader)), instances, options.arguments, options.useDefaultArguments);
    	}

		void Destroy() { rd->Destroy(); }

	    void SetCamera(const Math::Matrix3D& cam) { rd->camera = cam; }
	    void SetProjection(const Math::Matrix3D& proj) { rd->projection = proj; }
	    
	    void UseShader(Str code) { rd->shader = Shader::New(code); }
	    void UseShaderFromFile(CStr file) { rd->shader = Shader::FromFile(file); }
	    void UseShaderFromFile(CStr vert, CStr frag, CStr geom = {})
    	{ rd->shader = Shader::FromFile(vert, frag, geom); }
    };

    template <class T>
	void RenderObject<T>::Draw(IList<const Mesh<T>*> meshes, const DrawOptions& options) {
	    BeginContext();
    	for (auto* m : meshes) rd->Add(*m);
    	EndContext();
    	DrawContext(options);
    }

    template <class T>
	void RenderObject<T>::Draw(const CollectionAny auto& meshes, const DrawOptions& options) {
	    BeginContext();
    	AddMeshes(meshes);
    	EndContext();
    	DrawContext(options);
    }

    template <class T>
	void RenderObject<T>::DrawInstanced(IList<const Mesh<T>*> meshes, int instances, const DrawOptions& options) {
    	BeginContext();
    	for (auto* m : meshes) rd->Add(*m);
    	EndContext();
    	DrawContextInstanced(instances, options);
    }

	template <class T>
	void RenderObject<T>::DrawInstanced(const CollectionAny auto& meshes, int instances, const DrawOptions& options) {
    	BeginContext();
    	AddMeshes(meshes);
    	EndContext();
    	DrawContextInstanced(instances, options);
    }
}
