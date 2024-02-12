#include "Shader.h"

#include <fstream>
#include "stdu/io.h"

#include "Debugging.h"
#include "Texture.h"

namespace Graphics {
    glID ShaderHandler::Create() const {
        return GL_NULL;
    }

    void ShaderHandler::Destroy(glID id) const {
        GLCALL(glDeleteProgram(id));
    }

    void ShaderHandler::Bind(glID id) const {
        GLCALL(glUseProgram(id));
    }

    void ShaderHandler::Unbind() const {
        GLCALL(glUseProgram(0));
    }

    Shader::Shader(std::string_view program) {
        ShaderProgramSource shadersrc = ParseShader(program);
        rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
    }

    Shader::Shader(std::string_view vert, std::string_view frag) {
        rendererID = CreateShader(vert, frag);
    }

    uint Shader::GetUniformLocation(stringr name) {
        if (uniformCache.contains(name))
            return uniformCache[name];
    
        GLCALL(const int location = glGetUniformLocation(rendererID, name.c_str()));
        ASSERT(location != -1);
        uniformCache[name] = location;
        return location;
    }

    ShaderProgramSource Shader::ParseShader(std::string_view program) {
        uint lastLine = 0;
        Maths::urange ssv, ssf, *ss = nullptr;
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
        ss->max = program.size();

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
        GLCALL(glUniform1i(GetUniformLocation(name), texture.Slot()));
    }

    Shader Shader::FromFile(stringr filepath) {
        Shader s {};
        ShaderProgramSource shadersrc = ParseFromFile(filepath);
        s.rendererID = CreateShader(shadersrc.vertexShader, shadersrc.fragmentShader);
        return s;
    }
    
    uint Shader::CompileShader(std::string_view source, uint type) {
        const uint id = glCreateShader(type);
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
            DEBUGFUN(std::cout << "ERR: ERR WHEN COMPILING " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader." << msg << '\n');

            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    uint Shader::CreateShader(std::string_view vtx, std::string_view frg) {
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
