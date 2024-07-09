#pragma once
#include "RenderData.h"

#include "Utils/Ref.h"

namespace Quasi::Graphics {
	struct DrawOptions {
		Ref<Shader> shader = nullptr;
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

		void Draw(IList<const Mesh<T>*> meshes, const DrawOptions& options = {});
		void Draw(const Mesh<T>& mesh, const DrawOptions& options = {}) { Draw({ &mesh }, options); }
		template <CollectionOf<Mesh<T>> R> void Draw(R&& meshes, const DrawOptions& options = {});
		void DrawInstanced(IList<const Mesh<T>*> meshes, int instances, const DrawOptions& options = {});
    	void DrawInstanced(const Mesh<T>& mesh, int instances, const DrawOptions& options = {}) { DrawInstanced({ &mesh }, instances, options); }
    	template <CollectionOf<Mesh<T>> R> void DrawInstanced(R&& meshes, int instances, const DrawOptions& options = {});

    	void BeginContext() { rd->BufferUnload(); rd->Clear(); }
    	void AddMesh(const Mesh<T>& mesh) { rd->Add(mesh); }
    	template <CollectionOf<Mesh<T>> R> void AddMeshes(R&& meshes) { rd->Add(meshes); }
    	void AddMeshes(IList<const Mesh<T>*> meshes) { for (auto* m : meshes) rd->Add(*m); }
    	void EndContext() { rd->BufferLoad(); }

     	void DrawContext(const DrawOptions& options = {}) {
    		rd->Render(const_cast<Shader&>(options.shader.ValueOr(rd->shader)), options.arguments, options.useDefaultArguments);
    	}
    	void DrawContextInstanced(int instances, const DrawOptions& options = {}) {
    		rd->RenderInstanced(const_cast<Shader&>(options.shader.ValueOr(rd->shader)), instances, options.arguments, options.useDefaultArguments);
    	}

		void Destroy() { rd->Destroy(); rd = nullptr; }

	    void SetCamera(const Math::Matrix3D& cam) { rd->SetCamera(cam); }
	    void SetProjection(const Math::Matrix3D& proj) { rd->SetProjection(proj); }
	    
	    void UseShader(Str code) { rd->UseShader(code); }
	    void UseShaderFromFile(Str file) { rd->UseShaderFromFile(file); }
	    void UseShaderFromFile(Str vert, Str frag, Str geom = {})
    	{ rd->UseShaderFromFile(vert, frag, geom); }
    };

    template <class T>
	void RenderObject<T>::Draw(IList<const Mesh<T>*> meshes, const DrawOptions& options) {
	    BeginContext();
    	for (auto* m : meshes) rd->Add(*m);
    	EndContext();
    	DrawContext(options);
    }

    template <class T> template <CollectionOf<Mesh<T>> R>
	void RenderObject<T>::Draw(R&& meshes, const DrawOptions& options) {
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

	template <class T> template <CollectionOf<Mesh<T>> R>
	void RenderObject<T>::DrawInstanced(R&& meshes, int instances, const DrawOptions& options) {
    	BeginContext();
    	AddMeshes(meshes);
    	EndContext();
    	DrawContextInstanced(instances, options);
    }
}
