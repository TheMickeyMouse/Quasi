#pragma once

#include "Logger.h"

namespace Quasi::Graphics {
    // a very thin wrapper for an extern variable
    // (couldnt get them to work so here's an alternative)
    struct GLLoggerContainer {
        static Debug::Logger logger;
    };

    Debug::Logger& GLLogger();

    void InitGLLog();

    // taken from https://www.khronos.org/opengl/wiki/OpenGL_Error#Catching_errors_(the_hard_way)
    enum class GLErrorCode {
        NO_ERROR = 0,
        // no error
        INVALID_ENUM = 0x0500,
        // Given when an enumeration parameter is not a legal enumeration for that function. This is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
        INVALID_VALUE = 0x0501,
        // Given when a value parameter is not a legal value for that function. This is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
        INVALID_OPERATION = 0x0502,
        // Given when the set of state for a command is not legal for the parameters given to that command. It is also given for commands where combinations of parameters define what the legal parameters are.
        STACK_OVERFLOW = 0x0503,
        // Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size.
        STACK_UNDERFLOW = 0x0504,
        // Given when a stack popping operation cannot be done because the stack is already at its lowest point.
        OUT_OF_MEMORY = 0x0505,
        // Given when performing an operation that can allocate memory, and the memory cannot be allocated. The results of OpenGL functions that return this error are undefined; it is allowable for partial execution of an operation to happen in this circumstance.
        INVALID_FRAMEBUFFER_OPERATION = 0x0506,
        // Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete.
        CONTEXT_LOST = 0x0507, // (with OpenGL 4.5 or ARB_KHR_robustness)
        // Given if the OpenGL context has been lost, due to a graphics card reset.
        TABLE_TOO_LARGE = 0x8031,
        // Part of the ARB_imaging extension.
    };

    inline Q_ENUM_TOSTR(GLErrorCode, GLErrName,
        (NO_ERROR,                      "GL_NO_ERROR")
        (INVALID_ENUM,                  "GL_INVALID_ENUM")
        (INVALID_VALUE,                 "GL_INVALID_VALUE")
        (INVALID_OPERATION,             "GL_INVALID_OPERATION")
        (STACK_OVERFLOW,                "GL_STACK_OVERFLOW")
        (STACK_UNDERFLOW,               "GL_STACK_UNDERFLOW")
        (OUT_OF_MEMORY,                 "GL_OUT_OF_MEMORY")
        (INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION")
        (CONTEXT_LOST,                  "GL_CONTEXT_LOST")
        (TABLE_TOO_LARGE,               "GL_TABLE_TO_LARGE"),
    "GL_UNDEFINED_ERROR")

    GLErrorCode GLGetErr();
    void GLClearErr();
    void GLReport(const Debug::SourceLoc& loc = Debug::SourceLoc::current());
    void GLReportFn(Str signature, const Debug::SourceLoc& loc = Debug::SourceLoc::current());

    template <class F> auto GLCall(F&& f, Str fsig, const Debug::SourceLoc& loc = Debug::SourceLoc::current()) -> decltype(f()) {
        GLClearErr();
        if constexpr (std::is_void_v<decltype(f())>) {
            f();
            GLReportFn(fsig, loc);
            return;
        } else {
            auto res = f();
            GLReportFn(fsig, loc);
            return res;
        }
    }
#ifdef NDEBUG
#define Q_GL_CALL(X) (X)
#else
#define Q_GL_CALL(X) GLCall([&]{ return X; }, #X)
#endif
}

template <>
struct std::formatter<Quasi::Graphics::GLErrorCode> : std::formatter<Quasi::String> {
    auto format(Quasi::Graphics::GLErrorCode err, std::format_context& ctx) const {
        return std::formatter<Quasi::String>::format(std::format("0x{:04X} ({})", (int)err, GLErrName(err)), ctx);
    }
};