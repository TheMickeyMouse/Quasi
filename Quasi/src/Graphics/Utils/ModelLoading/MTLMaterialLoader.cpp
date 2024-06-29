#include "MTLMaterialLoader.h"
#include <ranges>

#include "OBJModel.h"

namespace Quasi::Graphics {
    void MTLMaterialLoader::Load(Str string) {
        ParseProperties(string);
        CreateMaterials();
    }

    void MTLMaterialLoader::ParseProperty(Str line) {
        const usize spaceIdx = line.find_first_of(' ');
        const Str prefix = line.substr(0, spaceIdx),
                  data   = line.substr(spaceIdx + 1);

#define MATCH_BEGIN if (false) {}
#define MATCH_PREFIX(M, T) else if (prefix == (M)) properties.emplace_back(CreateProperty<T>(data));
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
    }

    void MTLMaterialLoader::ParseProperties(Str string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(Str { line.begin(), line.end() });
        }
    }

    void MTLMaterialLoader::CreateMaterial(Span<const MTLProperty> matprop) {
        if (matprop.empty() || !matprop.front().Is<NewMaterial>()) return;

        materials.emplace_back();
        MTLMaterial& mat = materials.back();
        mat.name = matprop.front().As<NewMaterial>()->name;

#define MATCH_PROP(T, P, M) else if (const auto val_##T = prop.As<T>()) { mat.M = val_##T->P; }
        for (const MTLProperty& prop : matprop.subspan(1)) {
            if (false) {}
            MATCH_PROP(AmbientCol,       color,    Ka)
            MATCH_PROP(DiffuseCol,       color,    Kd)
            MATCH_PROP(SpecularCol,      color,    Ks)
            MATCH_PROP(EmissiveCol,      color,    Ke)
            MATCH_PROP(SpecularExp,      exp,      Ns)
            MATCH_PROP(OpticalDen,       density,  Ni)
            MATCH_PROP(Dissolve,         dissolve, d)
            MATCH_PROP(IlluminationType, itype,    illum)
        }
#undef MATCH_PROP
    }

    void MTLMaterialLoader::CreateMaterials() {
        u32 lastMat = 0;
        for (u32 i = 1; i < properties.size(); ++i) {
            if (!properties[i].Is<NewMaterial>()) continue;
            const Span<MTLProperty> mat = Span { properties }.subspan(lastMat, i - 1 - lastMat);
            lastMat = i;
            CreateMaterial(mat);
        }
        CreateMaterial(Span { properties }.subspan(lastMat));
    }

    String MTLMaterialLoader::DebugStr() const {
        String ss {};
#define MATCH_PROP(T, S, P) else if (const auto val_##T = p.As<T>()) { const auto& $ = val_##T; ss += S; ss += ": "; ss += P; }
        for (const MTLProperty& p : properties) {
            if (false) {}
            MATCH_PROP(NewMaterial,      "newmtl",       $->name)
            MATCH_PROP(AmbientCol,       "ambient",      $->color.hexcode())
            MATCH_PROP(DiffuseCol,       "diffuse",      $->color.hexcode())
            MATCH_PROP(SpecularCol,      "specular",     $->color.hexcode())
            MATCH_PROP(EmissiveCol,      "emmisive",     $->color.hexcode())
            MATCH_PROP(SpecularExp,      "specular exp", $->exp)
            MATCH_PROP(OpticalDen,       "optical den",  $->density)
            MATCH_PROP(Dissolve,         "dissolve",     $->dissolve)
            MATCH_PROP(IlluminationType, "illumtype",    std::to_string($->itype))
        }
        return ss;
    }
#undef MATCH_PROP
}
