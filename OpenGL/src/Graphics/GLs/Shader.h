#pragma once
#include <string>
#include <unordered_map>
#include "opengl.h"

#include "Debugging.h"
#include "GLObject.h"
#include "Matrix.h"

namespace Graphics {
    struct ShaderProgramSource {
        std::string_view vertexShader;
        std::string_view fragmentShader;
    };

    struct ShaderHandler : GLObjectHandler<ShaderHandler> {
        glID Create() const;
        void Destroy(glID id) const;
        void Bind(glID id) const;
        void Unbind() const;
    };

    class Shader : public GLObject<ShaderHandler> {
    private:
        std::unordered_map<std::string, int> uniformCache;
    public:
        Shader() = default;
        OPENGL_API explicit Shader(std::string_view program);
        OPENGL_API explicit Shader(std::string_view vert, std::string_view frag);

        using stringr = const std::string&; // the 'r' stands for reference
        using intptr = const int*;
        using uintptr = const uint*;
        using floatptr = const float*;

#pragma region Uniform Ints
#define SHADER_UNIF_I(n, ...) GLCALL(glUniform##n##i(GetUniformLocation(name), __VA_ARGS__))
        void SetUniform1I(stringr name, int val)                        { SHADER_UNIF_I(1, val); }
        void SetUniform2I(stringr name, int v0, int v1)                 { SHADER_UNIF_I(2, v0, v1); }
        void SetUniform2I(stringr name, intptr vals)                    { SHADER_UNIF_I(2, vals[0], vals[1]); }
        void SetUniform3I(stringr name, int v0, int v1, int v2)         { SHADER_UNIF_I(3, v0, v1, v2); }
        void SetUniform3I(stringr name, intptr vals)                    { SHADER_UNIF_I(3, vals[0], vals[1], vals[2]); }
        void SetUniform4I(stringr name, int v0, int v1, int v2, int v3) { SHADER_UNIF_I(4, v0, v1, v2, v3); }
        void SetUniform4I(stringr name, intptr vals)                    { SHADER_UNIF_I(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_I
#pragma endregion
#pragma region Uniform Unsigned Ints
#define SHADER_UNIF_UI(n, ...) GLCALL(glUniform##n##ui(GetUniformLocation(name), __VA_ARGS__))
        void SetUniform1IUnsigned(stringr name, uint val)                           { SHADER_UNIF_UI(1, val); }
        void SetUniform2IUnsigned(stringr name, uint v0, uint v1)                   { SHADER_UNIF_UI(2, v0, v1); }
        void SetUniform2IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(2, vals[0], vals[1]); }
        void SetUniform3IUnsigned(stringr name, uint v0, uint v1, uint v2)          { SHADER_UNIF_UI(3, v0, v1, v2); }
        void SetUniform3IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(3, vals[0], vals[1], vals[2]); }
        void SetUniform4IUnsigned(stringr name, uint v0, uint v1, uint v2, uint v3) { SHADER_UNIF_UI(4, v0, v1, v2, v3); }
        void SetUniform4IUnsigned(stringr name, uintptr vals)                       { SHADER_UNIF_UI(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_UI
#pragma endregion
#pragma region Uniform Int Vecs
#define SHADER_UNIF_IV(n, d, c) GLCALL(glUniform##n##iv(GetUniformLocation(name), c, d))
        void SetUniform1IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(1, vals, count); }
        void SetUniform2IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(2, vals, count); }
        void SetUniform3IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(3, vals, count); }
        void SetUniform4IVec(stringr name, intptr vals, uint count) { SHADER_UNIF_IV(4, vals, count); }
#undef SHADER_UNIF_IV
#pragma endregion
#pragma region Uniform Unsigned Int Vecs
#define SHADER_UNIF_UIV(n, d, c) GLCALL(glUniform##n##uiv(GetUniformLocation(name), c, d))
        void SetUniform1UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(1, vals, count); }
        void SetUniform2UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(2, vals, count); }
        void SetUniform3UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(3, vals, count); }
        void SetUniform4UIVec(stringr name, uintptr vals, uint count) { SHADER_UNIF_UIV(4, vals, count); }
#undef SHADER_UNIF_UIV
#pragma endregion
#pragma region Uniform Floats
#define SHADER_UNIF_F(n, ...) GLCALL(glUniform##n##f(GetUniformLocation(name), __VA_ARGS__))
        void SetUniform1F(stringr name, float val)                              { SHADER_UNIF_F(1, val); }
        void SetUniform2F(stringr name, float v0, float v1)                     { SHADER_UNIF_F(2, v0, v1); }
        void SetUniform2F(stringr name, floatptr vals)                          { SHADER_UNIF_F(2, vals[0], vals[1]); }
        void SetUniform3F(stringr name, float v0, float v1, float v2)           { SHADER_UNIF_F(3, v0, v1, v2); }
        void SetUniform3F(stringr name, floatptr vals)                          { SHADER_UNIF_F(3, vals[0], vals[1], vals[2]); }
        void SetUniform4F(stringr name, float v0, float v1, float v2, float v3) { SHADER_UNIF_F(4, v0, v1, v2, v3); }
        void SetUniform4F(stringr name, floatptr vals)                          { SHADER_UNIF_F(4, vals[0], vals[1], vals[2], vals[3]); }
#undef SHADER_UNIF_F
#pragma endregion
#pragma region Uniform Float Vecs
#define SHADER_UNIF_FV(n, d, c) GLCALL(glUniform##n##fv(GetUniformLocation(name), c, d))
        void SetUniform1FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(1, vals, count); }
        void SetUniform2FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(2, vals, count); }
        void SetUniform3FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(3, vals, count); }
        void SetUniform4FVec(stringr name, floatptr vals, uint count) { SHADER_UNIF_FV(4, vals, count); }
#undef SHADER_UNIF_FV
#pragma endregion
#pragma region Uniform Matricies
#define SHADER_UNIF_MAT(s, m) GLCALL(glUniformMatrix##s##fv(GetUniformLocation(name), 1, GL_FALSE, m)) // column major
        void SetUniformMatrix2x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(2, mat); }
        void SetUniformMatrix3x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(3, mat); }

        void SetUniformMatrix2x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(2x3, mat); }
        void SetUniformMatrix3x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(3x2, mat); }
        void SetUniformMatrix2x4(stringr name, floatptr mat) { SHADER_UNIF_MAT(2x4, mat); }
        void SetUniformMatrix4x2(stringr name, floatptr mat) { SHADER_UNIF_MAT(4x2, mat); }
        void SetUniformMatrix3x4(stringr name, floatptr mat) { SHADER_UNIF_MAT(3x4, mat); }
        void SetUniformMatrix4x3(stringr name, floatptr mat) { SHADER_UNIF_MAT(4x3, mat); }

        void SetUniformMatrix4x4(stringr name, const Maths::mat3D& mat) { SHADER_UNIF_MAT(4, mat.get_in_col()); }
#undef SHADER_UNIF_MAT
#pragma endregion
        OPENGL_API void SetUniformTex(stringr name, const class Texture& texture);

#define GLSL_SHADER(VERSION, V, F) "#shader vertex\n" "#version " #VERSION " core\n" STDU_TOSTR(STDU_REMOVE_SCOPE(V)) "\n#shader fragment\n" "#version " #VERSION " core\n" STDU_TOSTR(STDU_REMOVE_SCOPE(F))

        inline const static std::string StdColored =
            GLSL_SHADER(330,
                (
                    layout(location = 0) in vec4 position;
                    layout(location = 1) in vec4 color;
                    out vec4 v_color;
                    uniform mat4 u_projection;
                    uniform mat4 u_view;
                    void main() {
                        gl_Position = u_projection * u_view * position;
                        v_color = color;
                    }
                ),
                (
                    layout(location = 0) out vec4 color;
                    in vec4 v_color;
                    void main() {
                        color = v_color;
                    }
                )
            );

        inline const static std::string StdTextured =
            GLSL_SHADER(330,
                (
                    layout(location = 0) in vec4 position;
                    layout(location = 1) in vec4 color;
                    layout(location = 2) in vec2 texCoord;
                    out vec2 v_TexCoord;
                    out vec4 v_color;
                    uniform mat4 u_projection;
                    uniform mat4 u_view;
                    void main() {
                        gl_Position = u_projection * u_view * position;
                        v_color = color;
                        v_TexCoord = texCoord;
                    }
                ),
                (
                    layout(location = 0) out vec4 color;
                    in vec2 v_TexCoord;
                    in vec4 v_color;
                    uniform sampler2D u_Texture;
                    void main() {
                        vec4 texColor = texture(u_Texture, v_TexCoord);
                        color = v_color * texColor;
                    }
                )
            );

        OPENGL_API static Shader FromFile(stringr filepath);

    private:
        OPENGL_API uint GetUniformLocation(stringr name);
        OPENGL_API static ShaderProgramSource ParseShader(std::string_view program);
        OPENGL_API static ShaderProgramSource ParseFromFile(const std::string& filepath);
        OPENGL_API static uint CompileShader(std::string_view source, uint type);
        OPENGL_API static uint CompileShaderVert(std::string_view source) { return CompileShader(source, GL_VERTEX_SHADER); }
        OPENGL_API static uint CompileShaderFrag(std::string_view source) { return CompileShader(source, GL_FRAGMENT_SHADER); }
        OPENGL_API static uint CreateShader(std::string_view vtx, std::string_view frg);
    };
}
