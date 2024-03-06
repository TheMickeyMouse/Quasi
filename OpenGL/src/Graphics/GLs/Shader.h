#pragma once
#include <string>
#include <unordered_map>
#include <core.h>

#include "GLObject.h"
#include "Matrix.h"

namespace Graphics {
    enum class ShaderType {
        VERTEX = 0x8B31,
        FRAGMENT = 0x8B30,
        GEOMETRY = 0x8DD9,
        // all below requires OpenGL 4.3 above
        COMPUTE = 0x91B9,
        TESS_CONTROL = 0x8E88,
        TESS_EVALUATION = 0x8E87,
    };


    struct ShaderProgramSource {
        std::string_view vertexShader;
        std::string_view fragmentShader;
    };

    struct ShaderHandler : GLObjectHandler<ShaderHandler> {
        OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
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

        static STDU_ENUM_TOSTR(ShaderType, ShaderTypeName,
            (VERTEX,          "Vertex")
            (FRAGMENT,        "Fragment")
            (GEOMETRY,        "Geometry")
            (COMPUTE,         "Compute")
            (TESS_CONTROL,    "Tesselation Control")
            (TESS_EVALUATION, "Tesselation Eval"),
            "Undefined")
        OPENGL_API void SetUniform1I(stringr name, int val);
        OPENGL_API void SetUniform2I(stringr name, int v0, int v1);
        OPENGL_API void SetUniform2I(stringr name, intptr vals);
        OPENGL_API void SetUniform3I(stringr name, int v0, int v1, int v2);
        OPENGL_API void SetUniform3I(stringr name, intptr vals);
        OPENGL_API void SetUniform4I(stringr name, int v0, int v1, int v2, int v3);
        OPENGL_API void SetUniform4I(stringr name, intptr vals);
        OPENGL_API void SetUniform1IUnsigned(stringr name, uint val);
        OPENGL_API void SetUniform2IUnsigned(stringr name, uint v0, uint v1);
        OPENGL_API void SetUniform2IUnsigned(stringr name, uintptr vals);
        OPENGL_API void SetUniform3IUnsigned(stringr name, uint v0, uint v1, uint v2);
        OPENGL_API void SetUniform3IUnsigned(stringr name, uintptr vals);
        OPENGL_API void SetUniform4IUnsigned(stringr name, uint v0, uint v1, uint v2, uint v3);
        OPENGL_API void SetUniform4IUnsigned(stringr name, uintptr vals);
        OPENGL_API void SetUniform1IVec(stringr name, intptr vals, uint count);
        OPENGL_API void SetUniform2IVec(stringr name, intptr vals, uint count);
        OPENGL_API void SetUniform3IVec(stringr name, intptr vals, uint count);
        OPENGL_API void SetUniform4IVec(stringr name, intptr vals, uint count);
        OPENGL_API void SetUniform1UIVec(stringr name, uintptr vals, uint count);
        OPENGL_API void SetUniform2UIVec(stringr name, uintptr vals, uint count);
        OPENGL_API void SetUniform3UIVec(stringr name, uintptr vals, uint count);
        OPENGL_API void SetUniform4UIVec(stringr name, uintptr vals, uint count);
        OPENGL_API void SetUniform1F(stringr name, float val);
        OPENGL_API void SetUniform2F(stringr name, float v0, float v1);
        OPENGL_API void SetUniform2F(stringr name, floatptr vals);
        OPENGL_API void SetUniform3F(stringr name, float v0, float v1, float v2);
        OPENGL_API void SetUniform3F(stringr name, floatptr vals);
        OPENGL_API void SetUniform4F(stringr name, float v0, float v1, float v2, float v3);
        OPENGL_API void SetUniform4F(stringr name, floatptr vals);
        OPENGL_API void SetUniform1FVec(stringr name, floatptr vals, uint count);
        OPENGL_API void SetUniform2FVec(stringr name, floatptr vals, uint count);
        OPENGL_API void SetUniform3FVec(stringr name, floatptr vals, uint count);
        OPENGL_API void SetUniform4FVec(stringr name, floatptr vals, uint count);
        OPENGL_API void SetUniformMatrix2x2(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix3x3(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix2x3(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix3x2(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix2x4(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix4x2(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix3x4(stringr name, floatptr mat);
        OPENGL_API void SetUniformMatrix4x3(stringr name, floatptr mat);

        OPENGL_API void SetUniformMatrix4x4(stringr name, const Maths::mat3D& mat);
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
        OPENGL_API static Shader FromFile(stringr vert, stringr frag);

    private:
        OPENGL_API uint GetUniformLocation(stringr name);
        OPENGL_API static ShaderProgramSource ParseShader(std::string_view program);
        OPENGL_API static ShaderProgramSource ParseFromFile(const std::string& filepath);
        OPENGL_API static uint CompileShader(std::string_view source, ShaderType type);
        static uint CompileShaderVert(std::string_view source) { return CompileShader(source, ShaderType::VERTEX); }
        static uint CompileShaderFrag(std::string_view source) { return CompileShader(source, ShaderType::FRAGMENT); }
        OPENGL_API static uint CreateShader(std::string_view vtx, std::string_view frg);
    };
}

template <>
struct std::formatter<Graphics::ShaderType> : std::formatter<std::string> {
    auto format(Graphics::ShaderType s, std::format_context& ctx) const {
        return std::formatter<std::string>::format(Graphics::Shader::ShaderTypeName(s), ctx);
    }
};