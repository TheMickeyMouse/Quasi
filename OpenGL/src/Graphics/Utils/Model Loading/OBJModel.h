#pragma once
#include "Mesh.h"
#include "Vector.h"
#include "VertexElement.h"

namespace Graphics {
    using OBJVertex = VertexTextureNormal3D;

    struct MTLMaterial { // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        std::string name;
        // ambient, diffuse, specular, emissive color
        Maths::color3f Ka, Kd, Ks, Ke;
        // specular exponent, optical density
        float Ns, Ni;
        // dissolve
        float d;
        // illumination
        int illum;
    };

    struct OBJModel;
    struct OBJObject {
        std::string name;
        Mesh<OBJVertex> mesh;
        bool smoothShading;
        int materialIndex = -1;
        OBJModel* model;
    };

    struct OBJModel {
        std::vector<OBJObject> objects;
        std::vector<MTLMaterial> materials;

        OPENGL_API void BindMeshes(RenderObject<OBJVertex>& scene);
        OPENGL_API void AddMeshes(RenderObject<OBJVertex>& scene);

        OPENGL_API std::vector<Mesh<OBJVertex>> RetrieveMeshes();
    };
}
