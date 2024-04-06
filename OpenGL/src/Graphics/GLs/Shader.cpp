#include "Shader.h"

#include <numeric>

#include "stdu/io.h"

#include "GL/glew.h"
#include "GLDebug.h"
#include "Texture.h"

namespace Graphics {
    glID ShaderHandler::Create() const {
        return GL_NULL;
    }

    void ShaderHandler::Destroy(glID id) const {
        GL_CALL(glDeleteProgram(id));
    }

    void ShaderHandler::Bind(glID id) const {
        GL_CALL(glUseProgram(id));
    }

    void ShaderHandler::Unbind() const {
        GL_CALL(glUseProgram(0));
    }

    Shader::Shader(std::string_view program) {
        const ShaderProgramSource shadersrc = ParseShader(program);
        rendererID = CreateShader(
            shadersrc.GetShader(ShaderType::VERTEX),
            shadersrc.GetShader(ShaderType::FRAGMENT),
            shadersrc.GetShader(ShaderType::GEOMETRY)
        );
    }

    Shader::Shader(std::string_view vert, std::string_view frag, std::string_view geom) {
        rendererID = CreateShader(vert, frag, geom);
    }

    int Shader::GetUniformLocation(std::string_view name) {
        const auto it = uniformCache.find(name);
        if (it != uniformCache.end())
            return it->second;

        const int location = GL_CALL(glGetUniformLocation(rendererID, name.data()));
        GLLogger().AssertFmt(location != -1, {"Uniform location for '{}' was -1"}, name.substr(name.size() - 1));
        uniformCache[std::string { name }] = location;
        return location;
    }

    void Shader::SetUniformDyn(const ShaderParameter& arg) {
        using enum ShaderUniformType;
        const int loc = GetUniformLocation(arg.name);
        #define SWITCH_STATE(I) case UNIF_##I: SetUniformAtLoc<UNIF_##I>(loc, arg.value.as<ShaderUniformArgOf<UNIF_##I>>()); break;
        switch (arg.value.type) {
            SWITCH_STATE(1I)  SWITCH_STATE(2I)  SWITCH_STATE(3I)  SWITCH_STATE(4I)
            SWITCH_STATE(1UI) SWITCH_STATE(2UI) SWITCH_STATE(3UI) SWITCH_STATE(4UI)
            SWITCH_STATE(1F)  SWITCH_STATE(2F)  SWITCH_STATE(3F)  SWITCH_STATE(4F)
            SWITCH_STATE(1I_ARR)  SWITCH_STATE(2I_ARR)  SWITCH_STATE(3I_ARR)  SWITCH_STATE(4I_ARR)
            SWITCH_STATE(1UI_ARR) SWITCH_STATE(2UI_ARR) SWITCH_STATE(3UI_ARR) SWITCH_STATE(4UI_ARR)
            SWITCH_STATE(1F_ARR)  SWITCH_STATE(2F_ARR)  SWITCH_STATE(3F_ARR)  SWITCH_STATE(4F_ARR)
            SWITCH_STATE(MAT2x2) SWITCH_STATE(MAT2x3) SWITCH_STATE(MAT2x4)
            SWITCH_STATE(MAT3x2) SWITCH_STATE(MAT3x3) SWITCH_STATE(MAT3x4)
            SWITCH_STATE(MAT4x2) SWITCH_STATE(MAT4x3) SWITCH_STATE(MAT4x4)
            default:;
        }
    }

    void Shader::SetUniformArgs(const ShaderArgs& args) {
        for (const auto arg : args) {
            SetUniformDyn(arg);
        }
    }

    ShaderProgramSource Shader::ParseShader(std::string_view program) {
        uint lastLine = 0;
        Maths::rangez ssv, ssf, ssg, *ss = nullptr;
        for (uint i = 0; i < program.size(); ++i) {
            if (program[i] != '\n') continue;
            std::string_view line = std::string_view { program }.substr(lastLine, i - lastLine);
            lastLine = i + 1;

            if (line.find("#shader") != std::string::npos) {
                if (ss) ss->max = line.data() - program.data();
                if (line.find("vertex") != std::string::npos) {
                    ss = &ssv;
                } else if (line.find("fragment") != std::string::npos) {
                    ss = &ssf;
                } else if (line.find("geometry") != std::string::npos) {
                    ss = &ssg;
                }
                if (ss) ss->min = i + 1;
            }
        }
        if (ss) ss->max = program.size();

        std::string concatedProgram;
        concatedProgram.resize(ssv.width() + ssf.width() + ssg.width());
        std::copy(program.data() + ssv.min, program.data() + ssv.max, concatedProgram.data());
        std::copy(program.data() + ssf.min, program.data() + ssf.max, concatedProgram.data() + ssv.width());
        std::copy(program.data() + ssg.min, program.data() + ssg.max, concatedProgram.data() + ssv.width() + ssf.width());

        return {
            concatedProgram,
            { ssv.width(), ssv.width() + ssf.width() }
        };
    }


    ShaderProgramSource Shader::ParseFromFile(stringr filepath) {
        return ParseShader(stdu::readfile(filepath));
    }

    void Shader::SetUniformTex(stringr name, const Texture& texture) {
        SetUniformInt(name, texture.Slot());
    }

    Shader Shader::FromFile(stringr filepath) {
        Shader s {};
        const ShaderProgramSource shadersrc = ParseFromFile(filepath);
        s.rendererID = CreateShader(shadersrc.GetShader(ShaderType::VERTEX), shadersrc.GetShader(ShaderType::FRAGMENT), shadersrc.GetShader(ShaderType::GEOMETRY));
        return s;
    }

    Shader Shader::FromFile(stringr vert, stringr frag, stringr geom) {
        Shader s {};
        s.rendererID = CreateShader(stdu::readfile(vert), stdu::readfile(frag), stdu::readfile(geom));
        return s;
    }

    uint Shader::CompileShader(std::string_view source, ShaderType type) {
        const uint id = glCreateShader((int)type);
        const char* src = source.data();
        const int length = (int)source.length();
        glShaderSource(id, 1, &src, &length);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    
        if (result == GL_FALSE) {
            int len;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
            std::string msg;
            msg.resize(len - 1);
            glGetShaderInfoLog(id, len, &len, msg.data());
            GLLogger().Error({"Compiling {} shader yielded compiler errors:\n{}"}, type, msg);

            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    uint Shader::CreateShader(std::string_view vtx, std::string_view frg, std::string_view geo) {
        const uint program = glCreateProgram();
        const uint vs = CompileShaderVert(vtx);
        const uint fs = CompileShaderFrag(frg);
        const uint gm = geo.empty() ? 0 : CompileShaderGeom(geo);

        GL_CALL(glAttachShader(program, vs));
        GL_CALL(glAttachShader(program, fs));
        if (gm) GL_CALL(glAttachShader(program, gm));
        GL_CALL(glLinkProgram(program));
        GL_CALL(glValidateProgram(program));
        
        GL_CALL(glDeleteShader(vs));
        GL_CALL(glDeleteShader(fs));

        return program;
    }

    ShaderArgs::ShaderArgs(std::initializer_list<ShaderParameter> p) {
        nullSepName.reserve(
            std::accumulate(
                p.begin(), p.end(), usize { 0 },
                [](usize i, const ShaderParameter& sp) { return i + sp.name.size() + 1; }));
        params.reserve(p.size());

        usize idx = 0;
        for (const ShaderParameter& sp : p) {
            nullSepName += sp.name;
            nullSepName += '\0';
            params.emplace_back(Maths::rangez { idx, idx += sp.name.size() + 1 }, sp.value);
        }
    }

    ShaderValueVariant::ShaderValueVariant(const Texture& tex) {
        type = ShaderUniformType::UNIF_1I;
        datInt = tex.Slot();
        size = 1;
    }

#define DEFINE_UNIF_FN(IN, GL, ...) \
    template <>\
    OPENGL_API void Shader::SetUniformAtLoc<ShaderUniformType::UNIF_##IN>(int uniformLoc, ShaderUniformArgOf<ShaderUniformType::UNIF_##IN> val) { \
        GL_CALL(glUniform##GL(uniformLoc, __VA_ARGS__)); \
    }

    DEFINE_UNIF_FN(1I,  1i,  val) DEFINE_UNIF_FN(2I,  2i,  val.x, val.y) DEFINE_UNIF_FN(3I,  3i,  val.x, val.y, val.z) DEFINE_UNIF_FN(4I,  4i,  val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1UI, 1ui, val) DEFINE_UNIF_FN(2UI, 2ui, val.x, val.y) DEFINE_UNIF_FN(3UI, 3ui, val.x, val.y, val.z) DEFINE_UNIF_FN(4UI, 4ui, val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1F,  1f,  val) DEFINE_UNIF_FN(2F,  2f,  val.x, val.y) DEFINE_UNIF_FN(3F,  3f,  val.x, val.y, val.z) DEFINE_UNIF_FN(4F,  4f,  val.x, val.y, val.z, val.w)
    DEFINE_UNIF_FN(1I_ARR,  1iv,  (int)val.size(), val.data())
    DEFINE_UNIF_FN(2I_ARR,  2iv,  (int)val.size(), (const int*)  val.data())
    DEFINE_UNIF_FN(3I_ARR,  3iv,  (int)val.size(), (const int*)  val.data())
    DEFINE_UNIF_FN(4I_ARR,  4iv,  (int)val.size(), (const int*)  val.data())
    DEFINE_UNIF_FN(1UI_ARR, 1uiv, (int)val.size(), val.data())
    DEFINE_UNIF_FN(2UI_ARR, 2uiv, (int)val.size(), (const uint*) val.data())
    DEFINE_UNIF_FN(3UI_ARR, 3uiv, (int)val.size(), (const uint*) val.data())
    DEFINE_UNIF_FN(4UI_ARR, 4uiv, (int)val.size(), (const uint*) val.data())
    DEFINE_UNIF_FN(1F_ARR,  1fv,  (int)val.size(), val.data())
    DEFINE_UNIF_FN(2F_ARR,  2fv,  (int)val.size(), (const float*)val.data())
    DEFINE_UNIF_FN(3F_ARR,  3fv,  (int)val.size(), (const float*)val.data())
    DEFINE_UNIF_FN(4F_ARR,  4fv,  (int)val.size(), (const float*)val.data())

    DEFINE_UNIF_FN(MAT2x2, Matrix2fv,   (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT2x3, Matrix2x3fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT2x4, Matrix2x4fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT3x2, Matrix3x2fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT3x3, Matrix3fv,   (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT3x4, Matrix3x4fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT4x2, Matrix4x2fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT4x3, Matrix4x3fv, (int)val.size(), false, (const float*)val.data())
    DEFINE_UNIF_FN(MAT4x4, Matrix4fv,   (int)val.size(), false, (const float*)val.data())
}
