#include "GenericMesh.h"
#include "Mesh.h"

namespace Graphics {
	GenericMesh::~GenericMesh() {
		if (ptr)
			ptr->render = nullptr;
	}
    
	void GenericMesh::Transfer(GenericMesh& dest, GenericMesh&& from) {
		dest.ptr = from.ptr;
		from.ptr = nullptr;
	}

	void GenericMesh::Set(void* mesh) {
		ptr = (Mesh<singlebyte>*)mesh;
	}

	uint& GenericMesh::deviceIndex() { return ptr->deviceIndex; }
	uint GenericMesh::deviceIndex() const { return ptr->deviceIndex; }

	usize GenericMesh::vSizeBytes() const { return ptr->vertices.size(); }
	std::span<const byte> GenericMesh::vDataBytes() const { return stdu::span_cast<const byte>(std::span { ptr->vertices }); }
	usize GenericMesh::iSize() const { return ptr->indices.size(); }
}
