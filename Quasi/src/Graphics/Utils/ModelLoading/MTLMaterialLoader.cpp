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

        Qmatch (prefix, (
            case ("newmtl") properties.emplace_back(CreateProperty<NewMaterial>     (data));,
            case ("Ka")     properties.emplace_back(CreateProperty<AmbientCol>      (data));,
            case ("Kd")     properties.emplace_back(CreateProperty<DiffuseCol>      (data));,
            case ("Ks")     properties.emplace_back(CreateProperty<SpecularCol>     (data));,
            case ("Ke")     properties.emplace_back(CreateProperty<EmissiveCol>     (data));,
            case ("Ns")     properties.emplace_back(CreateProperty<SpecularExp>     (data));,
            case ("Ni")     properties.emplace_back(CreateProperty<OpticalDen>      (data));,
            case ("d")      properties.emplace_back(CreateProperty<Dissolve>        (data));,
            case ("illum")  properties.emplace_back(CreateProperty<IlluminationType>(data));
        ))
    }

    void MTLMaterialLoader::ParseProperties(Str string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(Str { line.begin(), line.end() });
        }
    }

    template <class MTL> MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty(Str data) {
        using namespace std::literals;
        Qmatch ((typename)MTL, (
            case (NewMaterial) return { NewMaterial { String { data } } };,
            if ((MTL m) { { m.color }; }) {
                const auto color = Math::fVector3::parse(data, " ", "", "");
                if (!color) return {};
                return { MTL { color->to_color3() } };
            },
            in (SpecularExp, OpticalDen, Dissolve) {
                const auto fnum = Text::Parse<float>(data);
                if (!fnum) return {};
                return { MTL { *fnum } };
            },
            case (IlluminationType) {
                const auto inum = Text::Parse<u32>(data);
                if (!inum) return {};
                return { IlluminationType { *inum } };
            },
            else return {};
        ))
    }
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::NewMaterial>     (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::AmbientCol>      (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::DiffuseCol>      (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::SpecularCol>     (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::EmissiveCol>     (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::SpecularExp>     (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::OpticalDen>      (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::Dissolve>        (Str data);
    template MTLMaterialLoader::MTLProperty MTLMaterialLoader::CreateProperty<MTLMaterialLoader::IlluminationType>(Str data);

    void MTLMaterialLoader::CreateMaterial(Span<const MTLProperty> matprop) {
        if (matprop.empty() || !matprop.front().Is<NewMaterial>()) return;

        materials.emplace_back();
        MTLMaterial& mat = materials.back();
        mat.name = matprop.front().As<NewMaterial>()->name;

        for (const MTLProperty& prop : matprop.subspan(1)) {
            prop.Visit(
                [&] (const AmbientCol&       x) { mat.Ka    = x.color;    },
                [&] (const DiffuseCol&       x) { mat.Kd    = x.color;    },
                [&] (const SpecularCol&      x) { mat.Ks    = x.color;    },
                [&] (const EmissiveCol&      x) { mat.Ke    = x.color;    },
                [&] (const SpecularExp&      x) { mat.Ns    = x.exp;      },
                [&] (const OpticalDen&       x) { mat.Ni    = x.density;  },
                [&] (const Dissolve&         x) { mat.d     = x.dissolve; },
                [&] (const IlluminationType& x) { mat.illum = x.itype;    },
                [] (const auto&) {}
            );
        }
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
        for (const MTLProperty& p : properties) {
            p.Visit(
                [&] (const NewMaterial&      x) { ss += "newmtl: ";       ss += x.name;                     },
                [&] (const AmbientCol&       x) { ss += "ambient: ";      ss += x.color.hexcode();          },
                [&] (const DiffuseCol&       x) { ss += "diffuse: ";      ss += x.color.hexcode();          },
                [&] (const SpecularCol&      x) { ss += "specular: ";     ss += x.color.hexcode();          },
                [&] (const EmissiveCol&      x) { ss += "emmisive: ";     ss += x.color.hexcode();          },
                [&] (const SpecularExp&      x) { ss += "specular exp: "; ss += std::to_string(x.exp);      },
                [&] (const OpticalDen&       x) { ss += "optical den: ";  ss += std::to_string(x.density);  },
                [&] (const Dissolve&         x) { ss += "dissolve: ";     ss += std::to_string(x.dissolve); },
                [&] (const IlluminationType& x) { ss += "illumtype: ";    ss += std::to_string(x.itype);    },
                [] (const auto&) {}
            );
        }
        return ss;
    }
}
