#pragma once
#include <cmath>
#include <ranges>
#include <vector>
#include <variant>

#include "MTLMaterialLoader.h"
#include "OBJModel.h"

#include "Vector.h"

namespace Quasi::Graphics {
    class OBJModelLoader {
    public:
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        struct MaterialLib { String dir;  };
        struct UseMaterial { String name; };
        struct Object      { String name; };
        struct Group       { String name; }; // not impl

        struct Vertex       { Math::fVector3 pos; };
        struct VertexTex    { Math::fVector2 tex; };
        struct VertexNormal { Math::fVector3 nrm; };
        struct VertexParam  { Math::fVector3 prm; }; // not impl

        struct Face { int indices[3][3]; };
        struct Line { Vec<int> indices; }; // not impl

        struct SmoothShade { bool enabled; };

        struct OBJProperty : Variant<
            MaterialLib, UseMaterial, Object, Group,
            Vertex, VertexTex, VertexNormal, VertexParam,
            Face, Line,
            SmoothShade> {};
    private:
        Vec<OBJProperty> properties;
        MTLMaterialLoader mats;
        OBJModel model;

        Vec<Math::fVector3> vertex;
        Vec<Math::fVector2> vertexTexture;
        Vec<Math::fVector3> vertexNormal;
        Vec<Face> faces;

        String folder, filename;
    public:
        OBJModelLoader() = default;

        void LoadFile(Str filepath);
        void Load(Str string);
        void LoadMaterialFile(Str filepath);
        void LoadMaterial(Str string);

        void ParseProperty(Str line);
        void ParseProperties(Str string);
        template <class T> static OBJProperty CreateProperty(Str data);

        void CreateModel();
        void CreateObject(Span<const OBJProperty> objprop);
        void ResolveObjectIndices(OBJObject& obj);

        OBJModel& GetModel() { return model; }
        [[nodiscard]] const OBJModel& GetModel() const { return model; }

        OBJModel&& RetrieveModel();

        [[nodiscard]] String DebugStr() const;
    };

    template <class OBJ> OBJModelLoader::OBJProperty OBJModelLoader::CreateProperty(Str data) {
        using namespace std::literals;
        if constexpr (std::is_same_v<OBJ, MaterialLib> ||
                      std::is_same_v<OBJ, UseMaterial> ||
                      std::is_same_v<OBJ, Object> ||
                      std::is_same_v<OBJ, Group>) {
            return { OBJ { String { data } } };
        } else if constexpr (std::is_same_v<OBJ, Line>) {
            Vec<int> indices;
            for (const auto idx : std::views::split(data, ' ')) {
                indices.push_back(Text::Parse<int>({ idx.begin(), idx.end() }).Or(-1));
            }
            return { Line { std::move(indices) } };
        } else if constexpr (std::is_same_v<OBJ, Face>) {
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
        } else if constexpr (std::is_same_v<OBJ, Vertex> ||
                             std::is_same_v<OBJ, VertexNormal> ||
                             std::is_same_v<OBJ, VertexParam>) {
            return { OBJ { Math::fVector3::parse(data, " ", "", "").ValueOr(Math::fVector3 { NAN }) } };
        } else if constexpr (std::is_same_v<OBJ, VertexTex>) {
            return { OBJ { Math::fVector2::parse(data, " ", "", "").ValueOr(Math::fVector2 { NAN }) } };
        } else if constexpr (std::is_same_v<OBJ, SmoothShade>) {
            return { SmoothShade { data != "0" } };
        } else return {};
    }
}
