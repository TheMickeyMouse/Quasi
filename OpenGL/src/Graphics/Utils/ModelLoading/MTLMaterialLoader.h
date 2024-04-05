#pragma once
#include <string>
#include <variant>
#include <vector>

#include "Vector.h"
#include "Color.h"
#include "stdu/io.h"

namespace Graphics {
    struct MTLMaterial;
}

namespace Graphics {
    class MTLMaterialLoader {
        public:
        enum class MTLPropertyType {
            NewMaterial,

            AmbientCol,
            DiffuseCol,
            SpecularCol,
            EmissiveCol,

            SpecularExp,
            OpticalDen,
            Dissolve,

            IlluminationType,

            None = -1,
        };

        struct MTLProperty {
            MTLPropertyType type;

            std::variant<
                std::string,
                Maths::color3f,
                int,
                float
            > data;

            MTLProperty() : type(MTLPropertyType::None) {}
        };
    private:
        std::vector<MTLProperty> properties;
        std::vector<MTLMaterial> materials; // TODO: use unordered_map for O(1)
    public:
        MTLMaterialLoader() = default;

        void LoadFile(const std::string& filename) { Load(stdu::readfile(filename)); }
        OPENGL_API void Load(std::string_view string);

        OPENGL_API void ParseProperty(std::string_view line);
        OPENGL_API void ParseProperties(std::string_view string);
        static MTLProperty CreateProperty(MTLPropertyType type, std::string_view data);

        OPENGL_API void CreateMaterial(std::span<const MTLProperty> matprop);
        OPENGL_API void CreateMaterials();

        [[nodiscard]] OPENGL_API std::string DebugStr() const;

        friend class OBJModelLoader;
    };
}
