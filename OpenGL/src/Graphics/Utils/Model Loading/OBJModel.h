#pragma once
#include "Mesh.h"
#include "Vector.h"
#include "VertexElement.h"

namespace Graphics {
    struct OBJVertex {
        Maths::fvec3 Position;
        Maths::fvec2 TextureCoordinate;
        Maths::fvec3 Normal;

        GL_VERTEX_T(OBJVertex);
        GL_VERTEX_FIELD((Position)(TextureCoordinate)(Normal));
        GL_VERTEX_CUSTOM_TRANSFORM(mat) {
            return {
                .Position = mat * Position,
                .TextureCoordinate = TextureCoordinate,
                .Normal = mat.inv().transpose() * Normal.with_w(0),
            };
        }
    };

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
