#include "OBJModelLoader.h"

#include <ranges>

namespace Quasi::Graphics {
    void OBJModelLoader::LoadFile(Str filepath) {
        std::tie(folder, filename) = Text::SplitDirectory(filepath);
        Load(Text::ReadFile(filepath).Assert());
    }

    void OBJModelLoader::Load(Str string) {
        ParseProperties(string);
        CreateModel();
    }

    void OBJModelLoader::LoadMaterialFile(Str filepath) {
        mats.LoadFile(folder + '\\' + String { filepath });
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::LoadMaterial(Str string) {
        mats.Load(string);
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::ParseProperty(const Str line) {
        const usize spaceIdx = line.find_first_of(' ');
        const Str prefix = line.substr(0, spaceIdx),
                  data   = line.substr(spaceIdx + 1);

        Qmatch (prefix, (
            case ("v")      properties.emplace_back(CreateProperty<Vertex>      (data));,
            case ("vt")     properties.emplace_back(CreateProperty<VertexTex>   (data));,
            case ("vn")     properties.emplace_back(CreateProperty<VertexNormal>(data));,
            case ("vp")     properties.emplace_back(CreateProperty<VertexParam> (data));,
            case ("f")      properties.emplace_back(CreateProperty<Face>        (data));,
            case ("l")      properties.emplace_back(CreateProperty<Line>        (data));,
            case ("o")      properties.emplace_back(CreateProperty<Object>      (data));,
            case ("g")      properties.emplace_back(CreateProperty<Group>       (data));,
            case ("s")      properties.emplace_back(CreateProperty<SmoothShade> (data));,
            case ("usemtl") properties.emplace_back(CreateProperty<UseMaterial> (data));,
            case ("mtllib") properties.emplace_back(CreateProperty<MaterialLib> (data));
        ))
    }

    void OBJModelLoader::ParseProperties(Str string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(Str { line.begin(), line.end() });
        }
    }

    template <class OBJ>
    OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty(Str data) {
        using namespace std::literals;
        Qmatch ((typename)OBJ, (
            in (MaterialLib, UseMaterial, Object, Group) { return { OBJ { String { data } } }; },
            in (Vertex, VertexNormal, VertexParam) {
                return { OBJ { Math::fVector3::parse(data, " ", "", "").ValueOr(Math::fVector3 { NAN }) } };
            },
            case (VertexTex) {
                return { OBJ { Math::fVector2::parse(data, " ", "", "").ValueOr(Math::fVector2 { NAN }) } };
            },
            case (Line) {
                Vec<int> indices;
                for (const auto idx : std::views::split(data, ' ')) {
                    indices.push_back(Text::Parse<int>({ idx.begin(), idx.end() }).Or(-1));
                }
                return { Line { std::move(indices) } };
            },
            case (Face) ({
                Face face;
                u32 i = 0;
                for (const auto idx : std::views::split(data, ' ')) {
                    if (i >= 3) return {};
                    const auto [v, t, n] = Math::iVector3::parse(Str { idx.begin(), idx.end() }, "/", "", "",
                        [](Str x) -> Option<int> { return Text::Parse<int>(x).ValueOr(-1); }).ValueOr({ -1 });
                    face.indices[i][0] = v; face.indices[i][1] = t; face.indices[i][2] = n;
                    ++i;
                }
                if (i != 3) return {};
                return { face };
            });,
            case (SmoothShade) { return { SmoothShade { data != "0" } }; },
            else return {};
        ))
    }
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::MaterialLib> (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::UseMaterial> (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::Object>      (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::Group>       (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::Vertex>      (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::VertexNormal>(Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::VertexParam> (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::VertexTex>   (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::Line>        (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::Face>        (Str data);
    template OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty<OBJModelLoader::SmoothShade> (Str data);

    void OBJModelLoader::CreateModel() {
        u32 lastObj = 0;
        for (u32 i = 0; i < properties.size(); ++i) {
            if (const auto matfile = properties[i].As<MaterialLib>()) {
                LoadMaterialFile(matfile->dir);
                continue;
            }
            if (!properties[i].Is<Object>()) continue;

            const auto objprop = Span { properties }.subspan(lastObj, i - lastObj);
            lastObj = i;
            CreateObject(objprop);
        }
        CreateObject(Span { properties }.subspan(lastObj));
    }

    void OBJModelLoader::CreateObject(Span<const OBJProperty> objprop) {
        if (objprop.empty() || !objprop.front().Is<Object>()) return;

        model.objects.emplace_back();
        OBJObject& object = model.objects.back();
        object.model = &model;
        object.name = objprop.front().As<Object>()->name;

        for (const OBJProperty& prop : objprop.subspan(1)) {
            prop.Visit(
                [&] (const UseMaterial& usemat) {
                    object.materialIndex = (int)(
                        std::ranges::find_if(model.materials,
                            [&](const MTLMaterial& m) { return m.name == usemat.name; })
                        - model.materials.begin()
                    );
                },
                [&] (const Vertex&       v) { vertex       .push_back(v.pos); },
                [&] (const VertexTex&    t) { vertexTexture.push_back(t.tex); },
                [&] (const VertexNormal& n) { vertexNormal .push_back(n.nrm); },
                [&] (const Face&         f) { faces.push_back(f); },
                [&] (SmoothShade        ss) { object.smoothShading = ss.enabled; },
                [] (const auto&) {}
            );
        }
        ResolveObjectIndices(object);
    }

    void OBJModelLoader::ResolveObjectIndices(OBJObject& obj) {
        struct Cmp3 {
            bool operator()(Math::iVector3 tripleA, Math::iVector3 tripleB) const {
                const auto [ax, ay, az] = tripleA;
                const auto [bx, by, bz] = tripleB;
                return ax != bx ? ax < bx : ay != by ? ay < by : az < bz;
            }
        };

        // a hashset is actually worse than a vector lol, cuz i need indices
        Vec<Math::iVector3> indices;
        for (const Face& f : faces) {
            indices.emplace_back(f.indices[0][0], f.indices[0][1], f.indices[0][2]);
            indices.emplace_back(f.indices[1][0], f.indices[1][1], f.indices[1][2]);
            indices.emplace_back(f.indices[2][0], f.indices[2][1], f.indices[2][2]);
        }
        std::ranges::sort(indices, Cmp3 {});
        const auto end = std::ranges::unique(indices).begin();
        const auto beg = indices.begin();
        obj.mesh.vertices.resize(end - beg);
        std::transform(beg, end, obj.mesh.vertices.begin(),
            [&](Math::uVector3 triple) {
                return OBJVertex {
                    triple.x == -1 ? Math::fVector3 {} : vertex[triple.x - 1],
                    triple.y == -1 ? Math::fVector2 {} : vertexTexture[triple.y - 1],
                    triple.z == -1 ? Math::fVector3 {} : vertexNormal[triple.z - 1]
                };
            });

        Vec<TriIndices>& ind = obj.mesh.indices;
        ind.reserve(faces.size() * 3);
        for (const Face& f : faces) {
            const auto i1 = std::lower_bound(beg, end, Math::iVector3 { f.indices[0][0], f.indices[0][1], f.indices[0][2] }, Cmp3 {}),
                       i2 = std::lower_bound(beg, end, Math::iVector3 { f.indices[1][0], f.indices[1][1], f.indices[1][2] }, Cmp3 {}),
                       i3 = std::lower_bound(beg, end, Math::iVector3 { f.indices[2][0], f.indices[2][1], f.indices[2][2] }, Cmp3 {});
            ind.emplace_back((u32)(i1 - beg), (u32)(i2 - beg), (u32)(i3 - beg));
        }

        faces.clear();
    }

    OBJModel&& OBJModelLoader::RetrieveModel() {
        return std::move(model);
    }

    String OBJModelLoader::DebugStr() const {
        String ss {};
        for (const OBJProperty& p : properties) {
            p.Visit(
                [&] (const Vertex&       x) { ss += "v: ";      ss += Text::Format(x.pos); },
                [&] (const VertexTex&    x) { ss += "vt: ";     ss += Text::Format(x.tex); },
                [&] (const VertexNormal& x) { ss += "vn: ";     ss += Text::Format(x.nrm); },
                [&] (const VertexParam&  x) { ss += "vp: ";     ss += Text::Format(x.prm); },
                [&] (const Face&         x) { ss += "f: ";      ss += Text::ArrStr(x.indices, ", ", [] (const int (&vtn)[3]) { return Text::ArrStr(vtn, "/"); }); },
                [&] (const Line&         x) { ss += "l: ";      ss += Text::ArrStr(x.indices); },
                [&] (const Object&       x) { ss += "o: ";      ss += x.name; },
                [&] (const Group&        x) { ss += "g: ";      ss += x.name; },
                [&] (const SmoothShade&  x) { ss += "s: ";      ss += x.enabled ? "true" : "false"; },
                [&] (const UseMaterial&  x) { ss += "usemtl: "; ss += x.name; },
                [&] (const MaterialLib&  x) { ss += "mtllib: "; ss += x.dir;  },
                [] (const auto&) {}
            );
        }
        ss += "========\n";
        ss += mats.DebugStr();
        return ss;
    }
}

