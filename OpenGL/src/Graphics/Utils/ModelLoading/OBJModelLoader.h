#pragma once
#include <vector>
#include <variant>

#include "MTLMaterialLoader.h"
#include "OBJModel.h"

#include "Vector.h"

namespace Graphics {
    class OBJModelLoader {
    public:
        enum class OBJPropertyType {
            MaterialLib,
            UseMaterial,
            Object,
            Group, // not used

            Vertex,
            VertexTex,
            VertexNormal,
            VertexParam, // not used

            Face,
            Line, // not used

            SmoothShade,

            None = -1,
        };

        struct TriFace {
            int v1, vt1, vn1,
                v2, vt2, vn2,
                v3, vt3, vn3;
        };

        struct OBJProperty {
            OBJPropertyType type;


            std::variant<
                std::string,
                Maths::fvec3,
                Maths::fvec2,
                TriFace,
                std::vector<int>,
                bool
            > data;
            OBJProperty() : type(OBJPropertyType::None) {}
        };
    private:
        std::vector<OBJProperty> properties;
        MTLMaterialLoader mats;
        OBJModel model;

        std::vector<Maths::fvec3> vertex;
        std::vector<Maths::fvec2> vertexTexture;
        std::vector<Maths::fvec3> vertexNormal;
        std::vector<TriFace> faces;

        std::string folder, filename;
    public:
        OBJModelLoader() = default;

        void LoadFile(const std::string& filepath);
        void Load(std::string_view string);
        void LoadMaterialFile(std::string_view filepath);
        void LoadMaterial(std::string_view string);

        void ParseProperty(std::string_view line);
        void ParseProperties(std::string_view string);
        static OBJProperty CreateProperty(OBJPropertyType type, std::string_view data);

        void CreateModel();
        void CreateObject(std::span<const OBJProperty> objprop);
        void ResolveObjectIndices(OBJObject& obj);

        OBJModel& GetModel() { return model; }
        [[nodiscard]] const OBJModel& GetModel() const { return model; }

        OBJModel&& RetrieveModel();

        [[nodiscard]] std::string DebugStr() const;
    };
}
