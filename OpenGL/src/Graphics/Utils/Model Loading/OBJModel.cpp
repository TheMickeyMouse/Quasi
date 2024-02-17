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
}
