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

#define MATCH_BEGIN if (false) {}
#define MATCH_PREFIX(M, T) else if (prefix == (M)) { properties.emplace_back(CreateProperty<T>(data)); }
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
    }

    void OBJModelLoader::ParseProperties(Str string) {
        using namespace std::literals;
        for (const auto line : std::views::split(string, "\r\n"sv)) {
            ParseProperty(Str { line.begin(), line.end() });
        }
    }

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
                [&] (SmoothShade ss) { object.smoothShading = ss.enabled; },
                [&] (const auto& _) {}
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
#define MATCH_PROP(T, S, P) else if (const auto val_##T = p.As<T>()) { ss += S; ss += ": "; const auto& $ = val_##T; ss += P; }
        for (const OBJProperty& p : properties) {
            if constexpr (false) {}
            MATCH_PROP(Vertex,       "v",  Text::Format($->pos))
            MATCH_PROP(VertexTex,    "vt", Text::Format($->tex))
            MATCH_PROP(VertexNormal, "vn", Text::Format($->nrm))
            MATCH_PROP(VertexParam,  "vp", Text::Format($->prm))
            MATCH_PROP(Face,         "f",  Text::ArrStr($->indices, ", ", [] (const int (&vtn)[3]) { return Text::ArrStr(vtn, "/"); }))
            MATCH_PROP(Line,         "l",  Text::ArrStr($->indices))
            MATCH_PROP(Object,       "o",  $->name)
            MATCH_PROP(Group,        "g",  $->name)
            MATCH_PROP(SmoothShade,  "s",  $->enabled ? "true" : "false")
            MATCH_PROP(UseMaterial,  "usemtl", $->name)
            MATCH_PROP(MaterialLib,  "mtllib", $->dir)
        }
        ss += "========\n";
        ss += mats.DebugStr();
        return ss;
    }
}

