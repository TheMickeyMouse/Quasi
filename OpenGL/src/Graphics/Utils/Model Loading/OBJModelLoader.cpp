#include "OBJModelLoader.h"

#include <array>
#include <variant>
#include <ranges>

#include "NumTypes.h"
#include "Parser.h"
#include "stdu/parsing.h"

namespace Graphics {
    void OBJModelLoader::LoadFile(const std::string& filepath) {
        std::tie(folder, filename) = stdu::getfolder(filepath);
        Load(stdu::readfile(filepath));
    }

    void OBJModelLoader::Load(std::string_view string) {
        ParseProperties(string);
        CreateModel();
    }

    void OBJModelLoader::LoadMaterialFile(std::string_view filepath) {
        mats.LoadFile(folder + std::string { filepath });
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::LoadMaterial(std::string_view string) {
        mats.Load(string);
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::ParseProperty(const std::string_view line) {
        const uint spaceIdx = line.find_first_of(' ');
        const std::string_view prefix = line.substr(0, spaceIdx),
                               data   = line.substr(spaceIdx + 1);

        OBJPropertyType type = OBJPropertyType::None;

#define MATCH_BEGIN if (false) {}
#define MATCH_PREFIX(M, T) else if (prefix == (M)) { type = OBJPropertyType::T; }
        MATCH_BEGIN
        MATCH_PREFIX("v",  Vertex)
        MATCH_PREFIX("vt", VertexTex)
        MATCH_PREFIX("vn", VertexNormal)
        MATCH_PREFIX("vp", VertexParam)

        MATCH_PREFIX("f",  Face)
        MATCH_PREFIX("l",  Line)
        MATCH_PREFIX("o",  Object)
        MATCH_PREFIX("g",  Group)

        MATCH_PREFIX("s",  SmoothShade)

        MATCH_PREFIX("usemtl", UseMaterial)
        MATCH_PREFIX("mtllib", MaterialLib)
#undef MATCH_BEGIN
#undef MATCH_PREFIX

        if (type == OBJPropertyType::None) return;

        properties.emplace_back(CreateProperty(type, data));
    }

    void OBJModelLoader::ParseProperties(std::string_view string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(std::string_view { line.begin(), line.end() });
        }
    }

    OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty(OBJPropertyType type, std::string_view data) {
        OBJProperty prop;
        prop.type = type;
        using namespace std::literals;
        switch (type) {
            case OBJPropertyType::MaterialLib:
            case OBJPropertyType::UseMaterial:
            case OBJPropertyType::Object:
            case OBJPropertyType::Group: {
                prop.data = std::string { data };
                return prop;
            }
            case OBJPropertyType::Line: {
                prop.data = std::vector<int> {};
                auto& list = std::get<std::vector<int>>(prop.data);
                uint lastSpace = 0;
                for (uint i = 0; i < data.size(); ++i) {
                    if (data[i] != ' ') continue;
                    const std::string_view lit = data.substr(lastSpace, i - lastSpace);
                    lastSpace = i + 1;
                    list.emplace_back(stdu::parse_int(lit));
                }
                return prop;
            }
            case OBJPropertyType::Face: {
                const auto arr = Maths::parse_vec<Maths::ivec3, 3>(
                    data,
                    [=](std::string_view v) { return Maths::parse_vec<int, 3>(v, "/"sv, ""sv, ""sv); },
                    " "sv, ""sv, ""sv)
                    .value_or(Maths::vec3<Maths::ivec3> { Maths::ivec3 { -1 } });
                prop.data = TriFace {
                    arr[0][0], arr[0][1], arr[0][2],
                    arr[1][0], arr[1][1], arr[1][2],
                    arr[2][0], arr[2][1], arr[2][2],
                };
                return prop;
            }
            case OBJPropertyType::Vertex:
            case OBJPropertyType::VertexNormal:
            case OBJPropertyType::VertexParam: {
                prop.data = Maths::parse_vec<float, 3>(data, " "sv, ""sv, ""sv).value_or(Maths::fvec3 { NAN });
                return prop;
            }
            case OBJPropertyType::VertexTex: {
                prop.data = Maths::parse_vec<float, 2>(data, " "sv, ""sv, ""sv).value_or(Maths::fvec2 { NAN });
                return prop;
            }
            case OBJPropertyType::SmoothShade:
                prop.data = (bool)stdu::parse_int(data);
                return prop;
            case OBJPropertyType::None:;
        }
        return {};
    }

    void OBJModelLoader::CreateModel() {
        uint lastObj = 0;
        for (uint i = 0; i < properties.size(); ++i) {
            if (properties[i].type == OBJPropertyType::MaterialLib) {
                LoadMaterialFile(std::get<std::string>(properties[i].data));
                continue;
            }

            if (properties[i].type != OBJPropertyType::Object) continue;
            const std::span objprop = std::span { properties }.subspan(lastObj, i - lastObj);
            lastObj = i;
            CreateObject(objprop);
        }
        CreateObject(std::span { properties }.subspan(lastObj));
    }

    void OBJModelLoader::CreateObject(std::span<const OBJProperty> objprop) {
        if (objprop.front().type != OBJPropertyType::Object) return;

        model.objects.emplace_back();
        OBJObject& object = model.objects.back();
        object.model = &model;
        object.name = std::get<std::string>(objprop.front().data);

        for (const OBJProperty& prop : objprop.subspan(1)) {
            switch (prop.type) {
                case OBJPropertyType::UseMaterial: {
                    const auto& matName = std::get<std::string>(prop.data);
                    object.materialIndex =
                        std::ranges::find_if(mats.materials,
                            [&](const MTLMaterial& m) { return m.name == matName; })
                        - mats.materials.begin();
                    break;
                }
                case OBJPropertyType::Vertex:
                    vertex.push_back(std::get<Maths::fvec3>(prop.data));
                    break;
                case OBJPropertyType::VertexTex:
                    vertexTexture.push_back(std::get<Maths::fvec2>(prop.data));
                    break;
                case OBJPropertyType::VertexNormal:
                    vertexNormal.push_back(std::get<Maths::fvec3>(prop.data));
                    break;
                case OBJPropertyType::Face:
                    faces.push_back(std::get<TriFace>(prop.data));
                    break;
                case OBJPropertyType::SmoothShade:
                    object.smoothShading = std::get<bool>(prop.data);
                    break;
                default:;
            }
        }
        ResolveObjectIndices(object);
    }

    void OBJModelLoader::ResolveObjectIndices(OBJObject& obj) {
        struct Cmp3 {
            bool operator()(Maths::uvec3 tripleA, Maths::uvec3 tripleB) const {
                const auto [ax, ay, az] = tripleA;
                const auto [bx, by, bz] = tripleB;
                return ax != bx ? ax < bx : ay != by ? ay < by : az < bz;
            }
        };

        // a hashset is actually worse than a vector lol, cuz i need indices
        std::vector<Maths::ivec3> indices;
        for (const TriFace& f : faces) {
            indices.emplace_back(f.v1, f.vt1, f.vn1);
            indices.emplace_back(f.v2, f.vt2, f.vn2);
            indices.emplace_back(f.v3, f.vt3, f.vn3);
        }
        std::sort(indices.begin(), indices.end(), Cmp3 {});
        const auto end = std::unique(indices.begin(), indices.end());
        const auto beg = indices.begin();
        obj.mesh.GetVertices().resize(end - beg);
        std::transform(beg, end, obj.mesh.GetVertices().begin(),
            [&](Maths::uvec3 triple) {
                return OBJVertex {
                    vertex[triple.x - 1],
                    vertexTexture[triple.y - 1],
                    vertexNormal[triple.z - 1]
                };
            });

        std::vector<TriIndices>& ind = obj.mesh.GetIndices();
        ind.reserve(faces.size() * 3);
        for (const TriFace& f : faces) {
            Maths::uvec3 v1 = { f.v1, f.vt1, f.vn1 },
                         v2 = { f.v2, f.vt2, f.vn2 },
                         v3 = { f.v3, f.vt3, f.vn3 };
            const auto i1 = std::lower_bound(beg, end, v1, Cmp3 {}),
                       i2 = std::lower_bound(beg, end, v2, Cmp3 {}),
                       i3 = std::lower_bound(beg, end, v3, Cmp3 {});
            ind.emplace_back(i1 - beg, i2 - beg, i3 - beg);
        }

        faces.clear();
    }

    std::string OBJModelLoader::DebugStr() const {
        std::stringstream ss {};
        for (const OBJProperty& p : properties) {
            switch (p.type) {
                case OBJPropertyType::MaterialLib:
                    ss << "mtllib, " << std::get<std::string>(p.data);
                    break;
                case OBJPropertyType::UseMaterial:
                    ss << "usemtl, " << std::get<std::string>(p.data);
                    break;
                case OBJPropertyType::Object:
                    ss << "object, " << std::get<std::string>(p.data);
                    break;
                case OBJPropertyType::Group:
                    ss << "group, " << std::get<std::string>(p.data);
                    break;
                case OBJPropertyType::Vertex:
                    ss << "position, " << std::get<Maths::fvec3>(p.data).str();
                    break;
                case OBJPropertyType::VertexTex:
                    ss << "texture, " << std::get<Maths::fvec2>(p.data).str();
                    break;
                case OBJPropertyType::VertexNormal:
                    ss << "normal, " << std::get<Maths::fvec3>(p.data).str();
                    break;
                case OBJPropertyType::VertexParam:
                    ss << "parameter, " << std::get<Maths::fvec3>(p.data).str();
                    break;
                case OBJPropertyType::Face: {
                    auto& face = std::get<TriFace>(p.data);
                    ss << "face, (";
                    ss << face.v1 << ',' << face.vt1 << ',' << face.vn1 << ")-(";
                    ss << face.v2 << ',' << face.vt2 << ',' << face.vn2 << ")-(";
                    ss << face.v3 << ',' << face.vt3 << ',' << face.vn3 << ")";
                    break;
                }
                case OBJPropertyType::Line:
                    ss << "line, ";
                    for (const int i : std::get<std::vector<int>>(p.data)) ss << std::to_string(i) << ',';
                    break;
                case OBJPropertyType::SmoothShade:
                    ss << "smooth shade, " + std::to_string(std::get<bool>(p.data));
                    break;
                case OBJPropertyType::None:
                    ss << "none, whoops lol";
                    break;
            }
            ss << '\n';
        }
        ss << "========\n";
        ss << mats.DebugStr();
        return ss.str();
    }
}

