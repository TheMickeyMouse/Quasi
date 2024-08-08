#include "GLDebug.h"

#include <glp.h>

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
        return GLErrorCode::Find(&GLErrorCodeData::glID, GL::GetError());
    }

    void GLClearErr() {
        while (GL::GetError()) {}
    }

    void GLReport(const Debug::SourceLoc& loc) {
        GLErrorCode err;
        while ((err = GLGetErr())) {
            GLLogger().Error({ "GL Error code {} was uncaught.", loc }, err);
        }
    }

    void GLReportFn(Str signature, const Debug::SourceLoc& loc) {
        GLErrorCode err;
        while ((err = GLGetErr())) {
            GLLogger().Error(
                { "GL Error code {} was uncaught when calling {} (see https://docs.gl/gl3/{}).", loc },
                err, signature, signature.substr(0, signature.find('(')));
        }
    }
}
