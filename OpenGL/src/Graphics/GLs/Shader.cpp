#include "Shader.h"

#include <fstream>
#include "stdu/io.h"

#include "Debugging.h"
#include "Texture.h"

namespace Graphics {
    Shader::Shader(const std::string& program) {
        ShaderProgramSource shadersrc = ParseShader(program);
        rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
    }

    Shader::Shader(const std::string& vert, const std::string& frag) {
        rendererID = CreateShader(vert, frag);
    }

    Shader::~Shader() {
        if (rendererID) {
            GLCALL(glDeleteProgram(rendererID));
        }
    }

    void Shader::Transfer(Shader& dest, Shader&& from) {
        dest.rendererID = from.rendererID;
        from.rendererID = 0;
        dest.uniformCache = std::move(from.uniformCache);
    }

    void Shader::Bind() const {
        GLCALL(glUseProgram(rendererID));
    }

    void Shader::Unbind() const {
        GLCALL(glUseProgram(0));
    }

    uint Shader::GetUniformLocation(stringr name) {
        if (uniformCache.contains(name))
            return uniformCache[name];
    
        GLCALL(const int location = glGetUniformLocation(rendererID, name.c_str()));
        ASSERT(location != -1);
        uniformCache[name] = location;
        return location;
    }

    ShaderProgramSource Shader::ParseShader(stringr program) {
        enum class ShaderType {
            NONE = -1, VERTEX = 0, FRAGMENT
        };

        std::stringstream prog { program };
        ShaderType type = ShaderType::NONE;
        std::string line;
        std::stringstream ss[2];
        while (std::getline(prog, line)) {
            if (line.find("#shader") != std::string::npos) {
                if (line.find("vertex") != std::string::npos) {
                    type = ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
                }
            }
            else {
                ss[(int)type] << line << "\n";
            }
        }

        return { ss[0].str(), ss[1].str() };
    }


    ShaderProgramSource Shader::ParseFromFile(stringr filepath) {
        return ParseShader(stdu::readfile(filepath));
    }

    void Shader::SetUniformTex(stringr name, const Texture& texture) {
        GLCALL(glUniform1i(GetUniformLocation(name), texture.Slot()));
    }

    Shader Shader::FromFile(stringr filepath) {
        Shader s {};
        ShaderProgramSource shadersrc = ParseFromFile(filepath);
        s.rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
        return s;
    }
    
    uint Shader::CompileShader(stringr source, uint type) {
        const uint id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    
        if (result == GL_FALSE) {
            int len;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
            char* msg = (char*)_malloca(len * sizeof(char));
            glGetShaderInfoLog(id, len, &len, msg);
            DEBUGFUN(std::cout << "ERR: ERR WHEN COMPILING " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader." << msg << '\n');

            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    uint Shader::CreateShader(stringr vtx, stringr frg) {
        const uint program = glCreateProgram();
        const uint vs = CompileShaderVert(vtx);
        const uint fs = CompileShaderFrag(frg);

        GLCALL(glAttachShader(program, vs));
        GLCALL(glAttachShader(program, fs));
        GLCALL(glLinkProgram(program));
        GLCALL(glValidateProgram(program));
        
        GLCALL(glDeleteShader(vs));
        GLCALL(glDeleteShader(fs));

        return program;
    }
}
