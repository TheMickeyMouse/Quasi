#include "Shader.h"

#include <fstream>
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
        rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
    }

    Shader::Shader(std::string_view vert, std::string_view frag) {
        rendererID = CreateShader(vert, frag);
    }

    uint Shader::GetUniformLocation(stringr name) {
        if (uniformCache.contains(name))
            return uniformCache[name];

        const int location = GL_CALL(glGetUniformLocation(rendererID, name.c_str()));
        GLLogger().AssertFmt(location != -1, {"Uniform location for '{}' was -1"}, name);
        uniformCache[name] = location;
        return location;
    }

    ShaderProgramSource Shader::ParseShader(std::string_view program) {
        uint lastLine = 0;
        Maths::rangez ssv, ssf, *ss = nullptr;
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
                } else continue;
                ss->min = i + 1;
            }
        }
        if (ss) ss->max = program.size();

        const ShaderProgramSource s = {
            std::string_view(program).substr(ssv.min, ssv.width()),
            std::string_view(program).substr(ssf.min, ssf.width())
        };
        return s;
    }


    ShaderProgramSource Shader::ParseFromFile(stringr filepath) {
        return ParseShader(stdu::readfile(filepath));
    }

    void Shader::SetUniformTex(stringr name, const Texture& texture) {
        GL_CALL(glUniform1i(GetUniformLocation(name), texture.Slot()));
    }

    Shader Shader::FromFile(stringr filepath) {
        Shader s {};
        const ShaderProgramSource shadersrc = ParseFromFile(filepath);
        s.rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
        return s;
    }

    Shader Shader::FromFile(stringr vert, stringr frag) {
        Shader s {};
        s.rendererID = CreateShader(stdu::readfile(vert), stdu::readfile(frag));
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
            char* msg = (char*)_malloca(len * sizeof(char));
            glGetShaderInfoLog(id, len, &len, msg);
            GLLogger().Error({"Compiling {} shader yielded compiler errors:\n{}"}, type, msg);

            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    uint Shader::CreateShader(std::string_view vtx, std::string_view frg) {
        const uint program = glCreateProgram();
        const uint vs = CompileShaderVert(vtx);
        const uint fs = CompileShaderFrag(frg);

        GL_CALL(glAttachShader(program, vs));
        GL_CALL(glAttachShader(program, fs));
        GL_CALL(glLinkProgram(program));
        GL_CALL(glValidateProgram(program));
        
        GL_CALL(glDeleteShader(vs));
        GL_CALL(glDeleteShader(fs));

        return program;
    }

    #pragma region Uniform Ints
#define SHADER_UNIF_I(n, ...) GL_CALL(glUniform##n##i(GetUniformLocation(name), __VA_ARGS__))
        void Shader::SetUniform1I(stringr name, int val)                        { SHADER_UNIF_I(1, val); }
        void Shader::SetUniform2I(stringr name, int v0, int v1)                 { SHADER_UNIF_I(2, v0, v1); }
        void Shader::SetUniform2I(stringr name, intptr vals)                    { SHADER_UNIF_I(2, vals[0], vals[1]); }
        void Shader::SetUniform3I(stringr name, int v0, int v1, int v2)         { SHADER_UNIF_I(3, v0, v1, v2); }
        void Shader::SetUniform3I(stringr name, intptr vals)                    { SHADER_UNIF_I(3, vals[0], vals[1], vals[2]); }
        void Shader::SetUniform4I(stringr name, int v0, int v1, int v2, int v3) { SHADER_UNIF_I(4, v0, v1, v2, v3); }
        void Shader::SetUniform4I(stringr name, intptr vals)                    { SHADER_UNIF_I(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_I
#pragma endregion
#pragma region Uniform Unsigned Ints
#define SHADER_UNIF_UI(n, ...) GL_CALL(glUniform##n##ui(GetUniformLocation(name), __VA_ARGS__))
        void Shader::SetUniform1IUnsigned(stringr name, uint val)                           { SHADER_UNIF_UI(1, val); }
        void Shader::SetUniform2IUnsigned(stringr name, uint v0, uint v1)                   { SHADER_UNIF_UI(2, v0, v1); }
        void Shader::SetUniform2IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(2, vals[0], vals[1]); }
        void Shader::SetUniform3IUnsigned(stringr name, uint v0, uint v1, uint v2)          { SHADER_UNIF_UI(3, v0, v1, v2); }
        void Shader::SetUniform3IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(3, vals[0], vals[1], vals[2]); }
        void Shader::SetUniform4IUnsigned(stringr name, uint v0, uint v1, uint v2, uint v3) { SHADER_UNIF_UI(4, v0, v1, v2, v3); }
        void Shader::SetUniform4IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_UI
#pragma endregion
#pragma region Uniform Int Vecs
#define SHADER_UNIF_IV(n, d, c) GL_CALL(glUniform##n##iv(GetUniformLocation(name), c, d))
        void Shader::SetUniform1IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(1, vals, count); }
        void Shader::SetUniform2IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(2, vals, count); }
        void Shader::SetUniform3IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(3, vals, count); }
        void Shader::SetUniform4IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(4, vals, count); }
#undef SHADER_UNIF_IV
#pragma endregion
#pragma region Uniform Unsigned Int Vecs
#define SHADER_UNIF_UIV(n, d, c) GL_CALL(glUniform##n##uiv(GetUniformLocation(name), c, d))
        void Shader::SetUniform1UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(1, vals, count); }
        void Shader::SetUniform2UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(2, vals, count); }
        void Shader::SetUniform3UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(3, vals, count); }
        void Shader::SetUniform4UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(4, vals, count); }
#undef SHADER_UNIF_UIV
#pragma endregion
#pragma region Uniform Floats
#define SHADER_UNIF_F(n, ...) GL_CALL(glUniform##n##f(GetUniformLocation(name), __VA_ARGS__))
        void Shader::SetUniform1F(stringr name, float val)                              { SHADER_UNIF_F(1, val); }
        void Shader::SetUniform2F(stringr name, float v0, float v1)                     { SHADER_UNIF_F(2, v0, v1); }
        void Shader::SetUniform2F(stringr name, floatptr vals)                          { SHADER_UNIF_F(2, vals[0], vals[1]); }
        void Shader::SetUniform3F(stringr name, float v0, float v1, float v2)           { SHADER_UNIF_F(3, v0, v1, v2); }
        void Shader::SetUniform3F(stringr name, floatptr vals)                          { SHADER_UNIF_F(3, vals[0], vals[1], vals[2]); }
        void Shader::SetUniform4F(stringr name, float v0, float v1, float v2, float v3) { SHADER_UNIF_F(4, v0, v1, v2, v3); }
        void Shader::SetUniform4F(stringr name, floatptr vals)                          { SHADER_UNIF_F(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_F
#pragma endregion
#pragma region Uniform Float Vecs
#define SHADER_UNIF_FV(n, d, c) GL_CALL(glUniform##n##fv(GetUniformLocation(name), c, d))
        void Shader::SetUniform1FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(1, vals, count); }
        void Shader::SetUniform2FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(2, vals, count); }
        void Shader::SetUniform3FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(3, vals, count); }
        void Shader::SetUniform4FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(4, vals, count); }
#undef SHADER_UNIF_FV
#pragma endregion
#pragma region Uniform Matricies
#define SHADER_UNIF_MAT(s, m) GL_CALL(glUniformMatrix##s##fv(GetUniformLocation(name), 1, GL_FALSE, m)) // column major
        void Shader::SetUniformMatrix2x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(2, mat); }
        void Shader::SetUniformMatrix3x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(3, mat); }

        void Shader::SetUniformMatrix2x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(2x3, mat); }
        void Shader::SetUniformMatrix3x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(3x2, mat); }
        void Shader::SetUniformMatrix2x4(stringr name, floatptr mat) { SHADER_UNIF_MAT(2x4, mat); }
        void Shader::SetUniformMatrix4x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(4x2, mat); }
        void Shader::SetUniformMatrix3x4(stringr name, floatptr mat) { SHADER_UNIF_MAT(3x4, mat); }
        void Shader::SetUniformMatrix4x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(4x3, mat); }

        void Shader::SetUniformMatrix4x4(stringr name, const Maths::mat3D& mat) { SHADER_UNIF_MAT(4, mat.get_in_col()); }
#undef SHADER_UNIF_MAT
#pragma endregion
}
