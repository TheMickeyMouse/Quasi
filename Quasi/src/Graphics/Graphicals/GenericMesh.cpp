#include "GenericMesh.h"
#include "Mesh.h"

namespace Quasi::Graphics {
	void GenericMesh::Set(void* mesh) {
		gmesh = DerefPtr((Mesh<singlebyte>*)mesh);
	}

	usize GenericMesh::vSizeBytes() const { return gmesh->vertices.size(); }
	Span<const byte> GenericMesh::vDataBytes() const { return CastSpan<const byte>(TakeSpan(gmesh->vertices)); }
	usize GenericMesh::iSize() const { return gmesh->indices.size(); }
}
