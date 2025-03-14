#include "OBJModelLoader.h"
#include "Utils/Algorithm.h"
#include "Utils/Comparison.h"

#include "Utils/Iter/MapIter.h"
#include "Utils/Match.h"
#include "Utils/Iter/LinesIter.h"
#include "Utils/Text/Parsing.h"
#include "Utils/Text/Num.h"

namespace Quasi::Graphics {
    void OBJModelLoader::LoadFile(CStr filepath) {
        Text::SplitDirectory(filepath).TieTo(folder, filename);
        Load(Text::ReadFile(filepath).Assert());
    }

    void OBJModelLoader::Load(Str string) {
        ParseProperties(string);
        CreateModel();
    }

    void OBJModelLoader::LoadMaterialFile(CStr filepath) {
        const usize len = folder.Length() + filepath.Length() + 2;
        char* fullpath = Memory::QAlloca$(char, len),
            * acc = fullpath;

        Memory::MemCopy(acc, folder.Data(), folder.Length()); acc += folder.Length();
        *acc++ = '\\';
        Memory::MemCopy(acc, filepath.Data(), filepath.Length());


        mats.LoadFile(CStr::SliceUnchecked(acc, len));
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::LoadMaterial(Str string) {
        mats.Load(string);
        model.materials = std::move(mats.materials);
    }

    void OBJModelLoader::ParseProperty(const Str line) {
        const usize spaceIdx = line.Find(' ');
        const auto [prefix, data] = line.SplitAt(spaceIdx);

        OBJProperty prop;
        Qmatch$ (prefix, (
            case ("v")      prop.Set(Vertex       { Math::fVector3::parse(data, " ", "", "").UnwrapOr(Math::fVector3 { Math::NaN }) });,
            case ("vt")     prop.Set(VertexTex    { Math::fVector2::parse(data, " ", "", "").UnwrapOr(Math::fVector2 { Math::NaN }) });,
            case ("vn")     prop.Set(VertexNormal { Math::fVector3::parse(data, " ", "", "").UnwrapOr(Math::fVector3 { Math::NaN }) });,
            case ("vp")     prop.Set(VertexParam  { Math::fVector3::parse(data, " ", "", "").UnwrapOr(Math::fVector3 { Math::NaN }) });,
            case ("f") ({
                Face face;
                u32 i = 0;
                for (const auto idx : data.Split(' ')) {
                    if (i >= 3) break;
                    const auto [v, t, n] = Math::iVector3::parse(idx, "/", "", "",
                        [](Str x) -> Option<int> { return Text::Parse<int>(x).UnwrapOr(-1); }).UnwrapOr({ -1 });
                    face.indices[i][0] = v; face.indices[i][1] = t; face.indices[i][2] = n;
                    ++i;
                }
                if (i == 3) prop.Set(face);
            });,
            case ("l") {
                Vec<int> indices;
                for (const auto idx : std::views::split(data, ' ')) {
                    indices.Push(Text::Parse<int>({ idx.begin(), idx.end() }).UnwrapOr(-1));
                }
                prop.Set<Line>({ indices });
            },
            case ("o")      prop.Set(Object { String(data) });,
            case ("g")      prop.Set(Group  { String(data) });,
            case ("s")      prop.Set(SmoothShade { data != "0" });,
            case ("usemtl") prop.Set(UseMaterial { String(data) });,
            case ("mtllib") {
                String mtllibdir = data;
                data += '\0';
                prop.Set(MaterialLib { std::move(mtllibdir) });
            }
        ))
        if (!prop.Is<Empty>())
            properties.Push(std::move(prop));
    }

    void OBJModelLoader::ParseProperties(Str string) {
        using namespace std::literals;
        for (const Str line : string.Lines()) {
            ParseProperty(line);
        }
    }

    void OBJModelLoader::CreateModel() {
        u32 lastObj = 0;
        for (u32 i = 0; i < properties.Length(); ++i) {
            if (const auto matfile = properties[i].As<MaterialLib>()) {
                LoadMaterialFile(CStr::FromUnchecked(matfile->dir));
                continue;
            }
            if (!properties[i].Is<Object>()) continue;

            const auto objprop = properties.Subspan(lastObj, i - lastObj);
            lastObj = i;
            CreateObject(objprop);
        }
        CreateObject(properties.Skip(lastObj));
    }

    void OBJModelLoader::CreateObject(Span<const OBJProperty> objprop) {
        if (objprop.IsEmpty() || !objprop[0].Is<Object>()) return;

        model.objects.Push({});
        OBJObject& object = model.objects.Last();
        object.model = &model;
        object.name = objprop[0].As<Object>()->name;

        for (const OBJProperty& prop : objprop.Skip(1)) {
            prop.Visit(
                [&] (const UseMaterial& usemat) {
                    object.materialIndex = (u32)model.materials.FindIf(
                        [&](const MTLMaterial& m) { return m.name == usemat.name; }
                    );
                },
                [&] (const Vertex&       v) { vertex       .Push(v.pos); },
                [&] (const VertexTex&    t) { vertexTexture.Push(t.tex); },
                [&] (const VertexNormal& n) { vertexNormal .Push(n.nrm); },
                [&] (const Face&         f) { faces.Push(f); },
                [&] (SmoothShade        ss) { object.smoothShading = ss.enabled; },
                [] (const auto&) {}
            );
        }
        ResolveObjectIndices(object);
    }

    void OBJModelLoader::ResolveObjectIndices(OBJObject& obj) {
        struct Cmp3 {
            Comparison operator()(Math::iVector3 tripleA, Math::iVector3 tripleB) const {
                const auto [ax, ay, az] = tripleA;
                const auto [bx, by, bz] = tripleB;
                return ax != bx ? Cmp::Between(ax, bx) : ay != by ? Cmp::Between(ay, by) : Cmp::Between(az, bz);
            }
        };

        // a hashset is actually worse than a vector lol, cuz i need indices
        Vec<Math::iVector3> indices;
        for (const Face& f : faces) {
            indices.Push({ f.indices[0][0], f.indices[0][1], f.indices[0][2] });
            indices.Push({ f.indices[1][0], f.indices[1][1], f.indices[1][2] });
            indices.Push({ f.indices[2][0], f.indices[2][1], f.indices[2][2] });
        }
        indices.SortBy(Cmp3 {});
        indices.RemoveDups();
        obj.mesh.vertices = indices.MapEach(
            [&] (Math::uVector3 triple) {
                return OBJVertex {
                    triple.x == -1 ? Math::fVector3 {} : vertex[triple.x - 1],
                    triple.y == -1 ? Math::fVector2 {} : vertexTexture[triple.y - 1],
                    triple.z == -1 ? Math::fVector3 {} : vertexNormal[triple.z - 1]
                };
            }
        );

        Vec<TriIndices>& ind = obj.mesh.indices;
        ind.Reserve(faces.Length() * 3);
        for (const Face& f : faces) {
            Math::iVector3 v1 { f.indices[0][0], f.indices[0][1], f.indices[0][2] };
            Math::iVector3 v2 { f.indices[1][0], f.indices[1][1], f.indices[1][2] };
            Math::iVector3 v3 { f.indices[2][0], f.indices[2][1], f.indices[2][2] };
            const usize i1 = indices.LowerBoundBy([&] (const Math::iVector3& x) { return Cmp3 {}(x, v1); }),
                        i2 = indices.LowerBoundBy([&] (const Math::iVector3& x) { return Cmp3 {}(x, v2); }),
                        i3 = indices.LowerBoundBy([&] (const Math::iVector3& x) { return Cmp3 {}(x, v3); });
            ind.Push({ (u32)i1, (u32)i2, (u32)i3 });
        }

        faces.Clear();
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

