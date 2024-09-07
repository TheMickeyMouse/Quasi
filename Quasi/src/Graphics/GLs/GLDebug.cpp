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

    GLErrorCode GLErrorCode::FromID(u32 glid) {
        return Find(&GLErrorCodeData::glID, glid);
    }

    u32 GLGetErr() {
        return GL::GetError();
    }

    void GLClearErr() {
        while (GL::GetError()) {}
    }

    void GLReport(const Debug::SourceLoc& loc) {
        u32 err;
        while ((err = GLGetErr())) {
            GLLogger().Error({ "GL Error code {} was uncaught.", loc }, GLErrorCode::FromID(err));
        }
    }

    void GLReportFn(Str signature, const Debug::SourceLoc& loc) {
        u32 err;
        while ((err = GLGetErr())) {
            GLLogger().Error(
                { "GL Error code {} was uncaught when calling {} (see https://docs.gl/gl3/{}).", loc },
                GLErrorCode::FromID(err), signature, signature.substr(0, signature.find('(')));
        }
    }
}
