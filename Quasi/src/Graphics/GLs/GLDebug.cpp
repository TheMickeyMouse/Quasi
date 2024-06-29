#include "GLDebug.h"

#include <GL/glew.h>

namespace Quasi::Graphics {
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

    void GLReportFn(Str signature, const Debug::SourceLoc& loc) {
        GLErrorCode err;
        while ((err = GLGetErr()) != GLErrorCode::NO_ERROR) {
            GLLogger().Error(
                { std::format_string<GLErrorCode&, Str&, Str> {"GL Error code {} was uncaught when calling {} (see https://docs.gl/gl3/{})."}, loc},
                err, signature, signature.substr(0, signature.find('(')));
        }
    }
}
