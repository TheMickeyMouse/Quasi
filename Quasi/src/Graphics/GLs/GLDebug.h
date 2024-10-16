#pragma once

#include "Logger.h"

namespace Quasi::Graphics {
    // a very thin wrapper for an extern variable
    // (couldnt get them to work so here's an alternative)
    struct GLDebugContainer {
        static Debug::Logger Logger;
        static Debug::TimeDuration GpuProcessDuration;
    };

    Debug::Logger& GLLogger();

    void InitGLDebugTools();

    struct GLErrorCodeData {
        u32 glID;
        Str errName;

        // taken from https://www.khronos.org/opengl/wiki/OpenGL_Error#Catching_errors_(the_hard_way)
        QDefineEnum$(GLErrorCode,
            // Given when an enumeration parameter is not a legal enumeration for that function.
            // This is given only for local problems; if the spec allows the enumeration in certain circumstances,
            // where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
            (INVALID_ENUM,                  (0x0500, "INVALID_ENUM"))
            // Given when a value parameter is not a legal value for that function.
            // This is only given for local problems; if the spec allows the value in certain circumstances,
            // where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.
            (INVALID_VALUE,                 (0x0501, "INVALID_VALUE"))
            // Given when the set of state for a command is not legal for the parameters given to that command.
            // It is also given for commands where combinations of parameters define what the legal parameters are.
            (INVALID_OPERATION,             (0x0502, "INVALID_OPERATION"))
            // Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size.
            (STACK_OVERFLOW,                (0x0503, "STACK_OVERFLOW"))
            // Given when a stack popping operation cannot be done because the stack is already at its lowest point.
            (STACK_UNDERFLOW,               (0x0504, "STACK_UNDERFLOW"))
            // Given when performing an operation that can allocate memory, and the memory cannot be allocated.
            // The results of OpenGL functions that return this error are undefined;
            // it is allowable for partial execution of an operation to happen in this circumstance.
            (OUT_OF_MEMORY,                 (0x0505, "OUT_OF_MEMORY"))
            // Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete.
            // (with OpenGL 4.5 or ARB_KHR_robustness)
            (INVALID_FRAMEBUFFER_OPERATION, (0x0506, "INVALID_FRAMEBUFFER_OPERATION"))
            // Given if the OpenGL context has been lost, due to a graphics card reset.
            (CONTEXT_LOST,                  (0x0507, "CONTEXT_LOST"))
            // Part of the ARB_imaging extension.
            (TABLE_TOO_LARGE,               (0x8031, "TABLE_TOO_LARGE")),
            NULLABLE, (0, "NO_ERROR")
        )

        static GLErrorCode FromID(u32 glid);
    };

    u32 GLGetErr();
    void GLClearErr();
    void GLReport(const Debug::SourceLoc& loc = Debug::SourceLoc::current());
    void GLReportFn(Str signature, const Debug::SourceLoc& loc = Debug::SourceLoc::current());

    template <class F> auto GLCall(F&& f, Str fsig, const Debug::SourceLoc& loc = Debug::SourceLoc::current()) -> decltype(f()) {
        GLClearErr();
        if constexpr (std::is_void_v<decltype(f())>) {
            const auto begin = Debug::Timer::Now();
            f();
            const Debug::DateTime end = Debug::Timer::Now();
            GLDebugContainer::GpuProcessDuration += end - begin;
            GLReportFn(fsig, loc);
            return;
        } else {
            const auto begin = Debug::Timer::Now();
            auto res = f();
            const Debug::DateTime end = Debug::Timer::Now();
            GLDebugContainer::GpuProcessDuration += end - begin;
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

namespace Quasi::Text {
    template <>
    struct Formatter<Graphics::GLErrorCode> {
        bool AddOption(Str) { return true; }
        void FormatTo(Graphics::GLErrorCode err, StringOutput output) const {
            FormatOnto(output,
                "0x{:X04} ({})",
                err->glID,
                err->errName
            );
        }
    };
}
