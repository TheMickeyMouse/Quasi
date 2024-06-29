#include "GenericMesh.h"
#include "Mesh.h"

namespace Quasi::Graphics {
	GenericMesh::~GenericMesh() {
		if (gmesh)
			gmesh->render = nullptr;
	}
    
	void GenericMesh::Transfer(GenericMesh& dest, GenericMesh&& from) {
		dest.gmesh = from.gmesh;
		from.gmesh = nullptr;
	}

	void GenericMesh::Set(void* mesh) {
		gmesh = DerefPtr((Mesh<singlebyte>*)mesh);
	}

	u32& GenericMesh::DeviceIndex() { return gmesh->deviceIndex; }
	u32 GenericMesh::DeviceIndex() const { return gmesh->deviceIndex; }
	Ref<RenderData>& GenericMesh::RenderParnet() { return gmesh->render; }
	Ref<RenderData>  GenericMesh::RenderParnet() const { return gmesh->render; }

	usize GenericMesh::vSizeBytes() const { return gmesh->vertices.size(); }
	Span<const byte> GenericMesh::vDataBytes() const { return CastSpan<const byte>(TakeSpan(gmesh->vertices)); }
	usize GenericMesh::iSize() const { return gmesh->indices.size(); }
}
