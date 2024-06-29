#include "OBJModel.h"

namespace Quasi::Graphics {
    void OBJModel::BindMeshes(RenderObject<OBJVertex>& scene) {
        for (OBJObject& o : objects) {
             scene.BindMesh(o.mesh);
        }
    }

    void OBJModel::AddMeshes(RenderObject<OBJVertex>& scene) {
        for (OBJObject& o : objects) {
            scene.AddNewMesh(o.mesh);
        }
    }

    Vec<Mesh<OBJVertex>> OBJModel::RetrieveMeshes() {
        Vec<Mesh<OBJVertex>> meshes;
        meshes.reserve(objects.size());
        for (OBJObject& obj : objects) {
            meshes.emplace_back(std::move(obj.mesh));
        }
        return meshes;
    }
}
