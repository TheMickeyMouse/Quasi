#pragma once
#include <vector>

#include "Math/Vector.h"
#include "Math/Color.h"
#include "Utils/Text.h"
#include "Utils/Variant.h"

namespace Quasi::Graphics {
    struct MTLMaterial;
}

namespace Quasi::Graphics {
    class MTLMaterialLoader {
        public:
        struct NewMaterial { String name; };
        struct AmbientCol  { Math::fColor3 color; };
        struct DiffuseCol  { Math::fColor3 color; };
        struct SpecularCol { Math::fColor3 color; };
        struct EmissiveCol { Math::fColor3 color; };
        struct SpecularExp { float exp; };
        struct OpticalDen  { float density; };
        struct Dissolve    { float dissolve; };
        struct IlluminationType { u32 itype; };

        struct MTLProperty : Variant<Empty,
            NewMaterial,
            AmbientCol, DiffuseCol, SpecularCol, EmissiveCol,
            SpecularExp, OpticalDen, Dissolve,
            IlluminationType
        > {};
    private:
        Vec<MTLProperty> properties;
        Vec<MTLMaterial> materials; // TODO: use unordered_map for O(1)
    public:
        MTLMaterialLoader() = default;

        void LoadFile(Str filename) { Load(Text::ReadFile(filename).Assert()); }
        void Load(Str string);

        void ParseProperty(Str line);
        void ParseProperties(Str string);

        template <class MTL>
        static MTLProperty CreateProperty(Str data);

        void CreateMaterial(Span<const MTLProperty> matprop);
        void CreateMaterials();

        [[nodiscard]] String DebugStr() const;

        friend class OBJModelLoader;
    };

    template <class MTL> MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty(Str data) {
        using namespace std::literals;
        if constexpr (std::is_same_v<MTL, NewMaterial>)
            return { NewMaterial { String { data } } };
        else if constexpr (requires (const MTL p) { { p.color } -> std::same_as<const Math::fColor3&>; }) {
            const auto color = Math::fVector3::parse(data, " ", "", "");
            if (!color) return {};
            return { MTL { color->to_color3() } };
        } else if constexpr (std::is_same_v<MTL, SpecularExp> ||
                      std::is_same_v<MTL, OpticalDen> ||
                      std::is_same_v<MTL, Dissolve>) {
            const auto fnum = Text::Parse<float>(data);
            if (!fnum) return {};
            return { MTL { *fnum } };
        } else if constexpr (std::is_same_v<MTL, IlluminationType>) {
            const auto inum = Text::Parse<u32>(data);
            if (!inum) return {};
            return { IlluminationType { *inum } };
        } else return {};
    }
}
