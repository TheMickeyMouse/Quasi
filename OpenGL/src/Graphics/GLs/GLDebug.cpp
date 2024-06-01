#include "GLDebug.h"

#include <GL/glew.h>

namespace Graphics {
    Debug::Logger GLLoggerContainer::logger { std::cout };

    Debug::Logger& GLLogger() {
        return GLLoggerContainer::logger;
    }

    void InitGLLog() {
        GLLogger().SetName("OpenGL");
        GLLogger().SetNameColor(Debug::ConsoleColor::FG_GREEN);
        GLLogger().SetBreakLevel(Debug::Severity::ERROR);
        GLLogger().SetShortenFile(true);
        GLLogger().SetIncludeFunc(false);
        GLLogger().SetAlwaysFlush(true);
        GLLogger().SetLocPad(0);
    }

    void GLInfo(const char* msg, const Debug::SourceLoc& loc) {
        GLLogger().Log(Debug::Severity::INFO, msg, loc);
    }

    void GLWarn(const char* msg, const Debug::SourceLoc& loc) {
        GLLogger().Log(Debug::Severity::WARN, msg, loc);
    }

    void GLError(const char* msg, const Debug::SourceLoc& loc) {
        GLLogger().Log(Debug::Severity::ERROR, msg, loc);
    }

    GLErrorCode GLGetErr() {
        return (GLErrorCode)glGetError();
    }

    void GLClearErr() {
        while (glGetError()) {}
    }

    void GLReport(const Debug::SourceLoc& loc) {
        GLErrorCode err;
        while ((err = GLGetErr()) != GLErrorCode::NO_ERROR) {
            GLLogger().Error({ std::format_string<GLErrorCode&> { "GL Error code {} was uncaught." }, loc}, err);
        }
    }

    void GLReportFn(std::string_view signature, const Debug::SourceLoc& loc) {
        GLErrorCode err;
        while ((err = GLGetErr()) != GLErrorCode::NO_ERROR) {
            GLLogger().Error(
                { std::format_string<GLErrorCode&, std::string_view&, std::string_view> {"GL Error code {} was uncaught when calling {} (see https://docs.gl/gl3/{})."}, loc},
                err, signature, signature.substr(0, signature.find('(')));
        }
    }
}
