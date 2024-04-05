#include "OBJModel.h"

namespace Graphics {
    void OBJModel::BindMeshes(RenderObject<OBJVertex>& scene) {
        for (OBJObject& o : objects) {
             scene.BindMeshes(o.mesh);
        }
    }

    void OBJModel::AddMeshes(RenderObject<OBJVertex>& scene) {
        for (OBJObject& o : objects) {
            scene.AddNewMeshes(o.mesh);
        }
    }

    std::vector<Mesh<OBJVertex>> OBJModel::RetrieveMeshes() {
        std::vector<Mesh<OBJVertex>> meshes;
        meshes.reserve(objects.size());
        for (OBJObject& obj : objects) {
            meshes.emplace_back(std::move(obj.mesh));
        }
        return meshes;
    }
}
