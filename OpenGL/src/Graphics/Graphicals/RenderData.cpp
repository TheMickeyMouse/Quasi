#include "RenderData.h"
#include "GraphicsDevice.h"

namespace Graphics {
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
		if (device) {
		    GraphicsDevice* gd = device;
		    device = nullptr;
		    gd->DeleteRender(deviceIndex);
		}
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

	void RenderData::ClearData(bool shallowClear) {
		vbo.ClearData(shallowClear);
		ibo.ClearData(shallowClear);
	}

	void RenderData::Render(Shader& replaceShader, const ShaderArgs& args, bool setDefaultShaderArgs) {
		device->Render(*this, replaceShader, args, setDefaultShaderArgs);
	}

	void RenderData::UnbindMesh(int index) {
		meshes.erase(meshes.begin() + index);
		UpdateMeshIndices();
	}

	void RenderData::UnbindMeshes(int indexStart, int indexEnd) {
		meshes.erase(meshes.begin() + indexStart, meshes.begin() + indexEnd);
		UpdateMeshIndices();
	}

	void RenderData::UpdateMeshIndices() {
		for (uint i = 0; i < meshes.size(); ++i)
			meshes[i].deviceIndex() = i;
	}

	void RenderData::Destroy() {
		GraphicsDevice* prev = device; // prevent infinte loop: deleterender -> erase renderdata -> destructor
		device = nullptr;
		prev->DeleteRender(deviceIndex);
		deviceIndex = 0;
	}

    void RenderData::EnableShader() {
	    shader.Bind();
	    shader.SetUniformMat4x4("u_projection", projection);
	    shader.SetUniformMat4x4("u_view", camera);
	}

    void RenderData::DisableShader() {
	    shader.Unbind();
	}
}