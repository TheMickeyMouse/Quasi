#include "MTLMaterialLoader.h"
#include <ranges>

#include "OBJModel.h"
#include "Parsing.h"

namespace Graphics {
    void MTLMaterialLoader::Load(std::string_view string) {
        ParseProperties(string);
        CreateMaterials();
    }

    void MTLMaterialLoader::ParseProperty(std::string_view line) {
        const usize spaceIdx = line.find_first_of(' ');
        const std::string_view prefix = line.substr(0, spaceIdx),
                               data   = line.substr(spaceIdx + 1);

        MTLPropertyType type = MTLPropertyType::None;

#define MATCH_BEGIN if (false) {}
#define MATCH_PREFIX(M, T) else if (prefix == (M)) { type = MTLPropertyType::T; }
        MATCH_BEGIN
        MATCH_PREFIX("newmtl", NewMaterial)

        MATCH_PREFIX("Ka", AmbientCol)
        MATCH_PREFIX("Kd", DiffuseCol)
        MATCH_PREFIX("Ks", SpecularCol)
        MATCH_PREFIX("Ke", EmissiveCol)

        MATCH_PREFIX("Ns", SpecularExp)
        MATCH_PREFIX("Ni", OpticalDen)
        MATCH_PREFIX("d", Dissolve)

        MATCH_PREFIX("illum", IlluminationType)
#undef MATCH_BEGIN
#undef MATCH_PREFIX

        if (type == MTLPropertyType::None) return;

        properties.emplace_back(CreateProperty(type, data));
    }

    void MTLMaterialLoader::ParseProperties(std::string_view string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(std::string_view { line.begin(), line.end() });
        }
    }

    MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty(MTLPropertyType type, std::string_view data) {
        MTLProperty prop;
        prop.type = type;
        using namespace std::literals;
        switch (type) {
            case MTLPropertyType::NewMaterial:
                prop.data = std::string { data };
            return prop;
            case MTLPropertyType::AmbientCol:
            case MTLPropertyType::DiffuseCol:
            case MTLPropertyType::SpecularCol:
            case MTLPropertyType::EmissiveCol: {
                const auto color = Maths::parse_vec<float, 3>(data, " "sv, ""sv, ""sv);
                if (!color.has_value()) return {};
                prop.data = color->color();
                return prop;
            }
            case MTLPropertyType::SpecularExp:
            case MTLPropertyType::OpticalDen:
            case MTLPropertyType::Dissolve:
                prop.data = stdu::parse_float(data);
            return prop;
            case MTLPropertyType::IlluminationType:
                prop.data = stdu::parse_int(data);
            return prop;
            case MTLPropertyType::None:;
        }
        return {};
    }

    void MTLMaterialLoader::CreateMaterial(std::span<const MTLProperty> matprop) {
        if (matprop.empty() || matprop.front().type != MTLPropertyType::NewMaterial) return;

        materials.emplace_back();
        MTLMaterial& mat = materials.back();
        mat.name = std::get<std::string>(matprop.front().data);

        for (const MTLProperty& prop : matprop.subspan(1)) {
            switch (prop.type) {
                case MTLPropertyType::AmbientCol:
                    mat.Ka = std::get<Maths::color3f>(prop.data);
                    break;
                case MTLPropertyType::DiffuseCol:
                    mat.Kd = std::get<Maths::color3f>(prop.data);
                    break;
                case MTLPropertyType::SpecularCol:
                    mat.Ks = std::get<Maths::color3f>(prop.data);
                    break;
                case MTLPropertyType::EmissiveCol:
                    mat.Ke = std::get<Maths::color3f>(prop.data);
                    break;
                case MTLPropertyType::SpecularExp:
                    mat.Ns = std::get<float>(prop.data);
                    break;
                case MTLPropertyType::OpticalDen:
                    mat.Ni = std::get<float>(prop.data);
                    break;
                case MTLPropertyType::Dissolve:
                    mat.d = std::get<float>(prop.data);
                    break;
                case MTLPropertyType::IlluminationType:
                    mat.illum = std::get<int>(prop.data);
                    break;
                default:;
            }
        }
    }

    void MTLMaterialLoader::CreateMaterials() {
        uint lastMat = 0;
        for (uint i = 1; i < properties.size(); ++i) {
            if (properties[i].type != MTLPropertyType::NewMaterial) continue;
            const std::span<MTLProperty> mat = std::span { properties }.subspan(lastMat, i - 1 - lastMat);
            lastMat = i;
            CreateMaterial(mat);
        }
        CreateMaterial(std::span { properties }.subspan(lastMat));
    }

    std::string MTLMaterialLoader::DebugStr() const {
        std::stringstream ss {};
        for (const MTLProperty& p : properties) {
            switch (p.type) {
                case MTLPropertyType::NewMaterial:
                    ss << "newmtl, " << std::get<std::string>(p.data);
                    break;
                case MTLPropertyType::AmbientCol:
                    ss << "ambient, " << std::get<Maths::color3f>(p.data).str();
                    break;
                case MTLPropertyType::DiffuseCol:
                    ss << "diffuse, " << std::get<Maths::color3f>(p.data).str();
                    break;
                case MTLPropertyType::SpecularCol:
                    ss << "specular, " << std::get<Maths::color3f>(p.data).str();
                    break;
                case MTLPropertyType::EmissiveCol:
                    ss << "emissive, " << std::get<Maths::color3f>(p.data).str();
                    break;
                case MTLPropertyType::SpecularExp:
                    ss << "specular exp, " << std::get<float>(p.data);
                    break;
                case MTLPropertyType::OpticalDen:
                    ss << "optical density, " << std::get<float>(p.data);
                    break;
                case MTLPropertyType::Dissolve:
                    ss << "dissolve, " << std::get<float>(p.data);
                    break;
                case MTLPropertyType::IlluminationType:
                    ss << "illum, " << std::get<int>(p.data);
                    break;
                case MTLPropertyType::None:;
            }
            ss << '\n';
        }
        return ss.str();
    }
}
