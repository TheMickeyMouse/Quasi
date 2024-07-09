#include "OBJModel.h"

namespace Quasi::Graphics {
    Vec<Mesh<OBJVertex>> OBJModel::RetrieveMeshes() {
        Vec<Mesh<OBJVertex>> meshes;
        meshes.reserve(objects.size());
        for (OBJObject& obj : objects) {
            meshes.emplace_back(std::move(obj.mesh));
        }
        return meshes;
    }
}
