#include "RenderData.h"
#include "GraphicsDevice.h"

namespace Quasi::Graphics {
	void RenderData::Transfer(RenderData& dest, RenderData&& from) {
		dest.varray = std::move(from.varray);
		dest.vbo = std::move(from.vbo);
		dest.ibo = std::move(from.ibo);

		dest.meshes = std::move(from.meshes);

		dest.device = from.device;
		from.device = nullptr;
		dest.deviceIndex = from.deviceIndex;
		from.deviceIndex = 0;
	}

	RenderData::~RenderData() {
		Destroy();

	}

	void RenderData::Bind() const {
		varray.Bind();
		vbo.Bind();
		ibo.Bind();
	}

	void RenderData::Unbind() const {
		varray.Unbind();
		vbo.Unbind();
		ibo.Unbind();
	}

	void RenderData::ClearData() {
		vbo.ClearData();
		ibo.ClearData();
	}

	void RenderData::Render(Shader& replaceShader, const ShaderArgs& args, bool setDefaultShaderArgs) {
		device->Render(*this, replaceShader, args, setDefaultShaderArgs);
	}

	void RenderData::RenderInstanced(Shader& replaceShader, int instances, const ShaderArgs& args, bool setDefaultShaderArgs) {
		device->RenderInstanced(*this, instances, replaceShader, args, setDefaultShaderArgs);
	}

	void RenderData::UnbindMesh(u32 index) {
		meshes.erase(meshes.begin() + index);
		UpdateMeshIndices();
	}

	void RenderData::UnbindMeshes(u32 indexStart, u32 indexEnd) {
		meshes.erase(meshes.begin() + indexStart, meshes.begin() + indexEnd);
		UpdateMeshIndices();
	}

	void RenderData::UpdateMeshIndices() {
		for (u32 i = 0; i < meshes.size(); ++i)
			meshes[i].DeviceIndex() = i;
	}

	void RenderData::Destroy() {
		if (device) {
			Ref<GraphicsDevice> prev = device; // prevent infinte loop: deleterender -> erase renderdata -> destructor
			device = nullptr;
			prev->DeleteRender(deviceIndex);
		}
	}
}