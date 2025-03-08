#include "Shader.h"

#include <glp.h>
#include "Graphics/Utils/Textures/Texture.h"
#include "Utils/Text.h"
#include "GLDebug.h"

namespace Quasi::Graphics {
    Shader::Shader(GraphicsID id) : GLObject(id) {}

    Shader Shader::New(Str program) {
        const ShaderProgramSource shadersrc = ParseShader(program);
        const GraphicsID rendererID = CreateShader(
            shadersrc.GetShader(ShaderType::VERTEX),
            shadersrc.GetShader(ShaderType::FRAGMENT),
            shadersrc.GetShader(ShaderType::GEOMETRY)
        );
        return Shader { rendererID };
    }

    Shader Shader::New(Str vert, Str frag, Str geom) {
        const GraphicsID rendererID = CreateShader(vert, frag, geom);
        return Shader { rendererID };
    }

    void Shader::DestroyObject(GraphicsID id) {
        QGLCall$(GL::DeleteProgram(id));
    }

    void Shader::BindObject(GraphicsID id) {
        QGLCall$(GL::UseProgram(id));
    }

    void Shader::UnbindObject() {
        QGLCall$(GL::UseProgram(0));
    }

    int Shader::GetUniformLocation(Str name) {
        const auto it = uniformCache.find(name);
        if (it != uniformCache.end())
            return it->second;

        const int location = QGLCall$(GL::GetUniformLocation(rendererID, name.Data()));
        GLLogger().Assert(location != -1, "invalid uniform location for '{}'", name);
        uniformCache[String { name }] = location;
        return location;
    }

    void Shader::SetUniformDyn(const ShaderParameter& arg) {
        using enum ShaderUniformType;
        const int loc = GetUniformLocation(arg.name);
        #define SWITCH_STATE(I) case UNIF_##I: SetUniformAtLoc<UNIF_##I>(loc, arg.value.as<ShaderUniformArgOf<UNIF_##I>>()); break;
        switch (arg.value.type) {
            SWITCH_STATE(1I)      SWITCH_STATE(2I)      SWITCH_STATE(3I)      SWITCH_STATE(4I)
            SWITCH_STATE(1UI)     SWITCH_STATE(2UI)     SWITCH_STATE(3UI)     SWITCH_STATE(4UI)
            SWITCH_STATE(1F)      SWITCH_STATE(2F)      SWITCH_STATE(3F)      SWITCH_STATE(4F)
            SWITCH_STATE(1I_ARR)  SWITCH_STATE(2I_ARR)  SWITCH_STATE(3I_ARR)  SWITCH_STATE(4I_ARR)
            SWITCH_STATE(1UI_ARR) SWITCH_STATE(2UI_ARR) SWITCH_STATE(3UI_ARR) SWITCH_STATE(4UI_ARR)
            SWITCH_STATE(1F_ARR)  SWITCH_STATE(2F_ARR)  SWITCH_STATE(3F_ARR)  SWITCH_STATE(4F_ARR)
            SWITCH_STATE(MAT2x2)  SWITCH_STATE(MAT2x3)  SWITCH_STATE(MAT2x4)
            SWITCH_STATE(MAT3x2)  SWITCH_STATE(MAT3x3)  SWITCH_STATE(MAT3x4)
            SWITCH_STATE(MAT4x2)  SWITCH_STATE(MAT4x3)  SWITCH_STATE(MAT4x4)
            default:;
        }
    }

    void Shader::SetUniformArgs(const ShaderArgs& args) {
        for (const auto arg : args) {
            SetUniformDyn(arg);
        }
    }

    ShaderProgramSource Shader::ParseShader(Str program) {
        usize lastLine = 0;
        Math::zRange ssv, ssf, ssg, *ss = nullptr;
        for (usize i = 0; i < program.Length(); ++i) {
            if (program[i] != '\n') continue;
            Str line = program.Substr(lastLine, i - lastLine);
            lastLine = i + 1;

            if (line.StartsWith("#shader")) {
                if (ss) ss->max = program.Unaddress(line.Data());

                line.Advance(8);

                if (line.StartsWith("vertex"))
                    ss = &ssv;
                else if (line.StartsWith("fragment"))
                    ss = &ssf;
                else if (line.StartsWith("geometry"))
                    ss = &ssg;

                if (ss) ss->min = i + 1;
            }
        }
        if (ss) ss->max = program.Length();

        String concatedProgram = String::WithCap(ssv.width() + ssf.width() + ssg.width());
        concatedProgram += program.Substr(ssv.min, ssv.width());
        concatedProgram += program.Substr(ssv.min, ssv.width());
        concatedProgram += program.Substr(ssv.min, ssv.width());

        return {
            concatedProgram,
            { ssv.width(), ssv.width() + ssf.width() }
        };
    }


    ShaderProgramSource Shader::ParseFromFile(CStr filepath) {
        return ParseShader(Text::ReadFile(filepath).Assert());
    }

    void Shader::SetUniformTex(Str name, const Texture& texture) {
        SetUniformInt(name, texture.Slot());
    }

    Shader Shader::FromFile(CStr filepath) {
        Shader s {};
        const ShaderProgramSource shadersrc = ParseFromFile(filepath);
        s.rendererID = CreateShader(shadersrc.GetShader(ShaderType::VERTEX), shadersrc.GetShader(ShaderType::FRAGMENT), shadersrc.GetShader(ShaderType::GEOMETRY));
        return s;
    }

    Shader Shader::FromFile(CStr vert, CStr frag, CStr geom) {
        Shader s {};
        s.rendererID = CreateShader(
            Text::ReadFile(vert).Assert(),
            Text::ReadFile(frag).Assert(),
            geom.IsEmpty() ? "" : Text::ReadFile(geom).Assert()
        );
        return s;
    }

    GraphicsID Shader::CompileShader(Str source, ShaderType type) {
        const GraphicsID id = GL::CreateShader(type->glID);
        const char* src = source.Data();
        const int length = (int)source.Length();
        GL::ShaderSource(id, 1, &src, &length);
        GL::CompileShader(id);

        int result;
        GL::GetShaderiv(id, GL::COMPILE_STATUS, &result);
    
        if (result == 0) {
            int len;
            GL::GetShaderiv(id, GL::INFO_LOG_LENGTH, &len);
            char* errbuf = Memory::QAlloca$(char, len);
            GL::GetShaderInfoLog(id, len, &len, errbuf);
            GLLogger().Error("Compiling {} shader yielded compiler errors:\n{}", type->shaderName, errbuf);

            GL::DeleteShader(id);
            return 0;
        }

        return id;
    }

    GraphicsID Shader::CreateShader(Str vtx, Str frg, Str geo) {
        const GraphicsID program = GL::CreateProgram();
        const GraphicsID vs = CompileShaderVert(vtx);
        const GraphicsID fs = CompileShaderFrag(frg);
        const GraphicsID gm = geo.IsEmpty() ? 0 : CompileShaderGeom(geo);

                QGLCall$(GL::AttachShader(program, vs));
                QGLCall$(GL::AttachShader(program, fs));
        if (gm) QGLCall$(GL::AttachShader(program, gm));

        QGLCall$(GL::LinkProgram(program));
        QGLCall$(GL::ValidateProgram(program));
        
                QGLCall$(GL::DeleteShader(vs));
                QGLCall$(GL::DeleteShader(fs));
        if (gm) QGLCall$(GL::DeleteShader(fs));

        return program;
    }

    ShaderArgs::ShaderArgs(IList<ShaderParameter> p) : params(Vec<ShaderValueVariant>::WithCap(p.size())) {
        for (const auto param : p) {
            args.Push(param.name);
            params.Push(param.value);
        }
    }

    ShaderValueVariant::ShaderValueVariant(const Texture& tex) : ShaderValueVariant(tex.Slot()) {}

#define DEFINE_UNIF_FN(IN, S, ...) \
    template <>\
    void Shader::SetUniformAtLoc<ShaderUniformType::UNIF_##IN>(int uniformLoc, ShaderUniformArgOf<ShaderUniformType::UNIF_##IN> val) { \
        QGLCall$(GL::Uniform##S(uniformLoc, __VA_ARGS__)); \
    }

    DEFINE_UNIF_FN(1I,  1i,  val) DEFINE_UNIF_FN(2I,  2i,  val.x, val.y) DEFINE_UNIF_FN(3I,  3i,  val.x, val.y, val.z) DEFINE_UNIF_FN(4I,  4i,  val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1UI, 1ui, val) DEFINE_UNIF_FN(2UI, 2ui, val.x, val.y) DEFINE_UNIF_FN(3UI, 3ui, val.x, val.y, val.z) DEFINE_UNIF_FN(4UI, 4ui, val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1F,  1f,  val) DEFINE_UNIF_FN(2F,  2f,  val.x, val.y) DEFINE_UNIF_FN(3F,  3f,  val.x, val.y, val.z) DEFINE_UNIF_FN(4F,  4f,  val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1I_ARR,  1iv,  (int)val.Length(), val.Data())
    DEFINE_UNIF_FN(2I_ARR,  2iv,  (int)val.Length(), Memory::TransmutePtr<const int>(val.Data()))
    DEFINE_UNIF_FN(3I_ARR,  3iv,  (int)val.Length(), Memory::TransmutePtr<const int>(val.Data()))
    DEFINE_UNIF_FN(4I_ARR,  4iv,  (int)val.Length(), Memory::TransmutePtr<const int>(val.Data()))
    DEFINE_UNIF_FN(1UI_ARR, 1uiv, (int)val.Length(), val.Data())
    DEFINE_UNIF_FN(2UI_ARR, 2uiv, (int)val.Length(), Memory::TransmutePtr<const uint> (val.Data()))
    DEFINE_UNIF_FN(3UI_ARR, 3uiv, (int)val.Length(), Memory::TransmutePtr<const uint> (val.Data()))
    DEFINE_UNIF_FN(4UI_ARR, 4uiv, (int)val.Length(), Memory::TransmutePtr<const uint> (val.Data()))
    DEFINE_UNIF_FN(1F_ARR,  1fv,  (int)val.Length(), val.Data())
    DEFINE_UNIF_FN(2F_ARR,  2fv,  (int)val.Length(), Memory::TransmutePtr<const float>(val.Data()))
    DEFINE_UNIF_FN(3F_ARR,  3fv,  (int)val.Length(), Memory::TransmutePtr<const float>(val.Data()))
    DEFINE_UNIF_FN(4F_ARR,  4fv,  (int)val.Length(), Memory::TransmutePtr<const float>(val.Data()))

    DEFINE_UNIF_FN(MAT2x2, Matrix2fv,   (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT2x3, Matrix2x3fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT2x4, Matrix2x4fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT3x2, Matrix3x2fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT3x3, Matrix3fv,   (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT3x4, Matrix3x4fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT4x2, Matrix4x2fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT4x3, Matrix4x3fv, (int)val.Length(), false, (const float*)val.Data())
    DEFINE_UNIF_FN(MAT4x4, Matrix4fv,   (int)val.Length(), false, (const float*)val.Data())
}
