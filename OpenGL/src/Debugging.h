#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <GL/glew.h>

#define DEBUG 1
#define USE_DETAILED_ERRORS 1
#define FORCE_BREAKPOINTS 1

#if DEBUG == 1

static std::string IntToErrCode(unsigned int err) {
    std::stringstream stream;
    stream << "0x"
        << std::setfill('0') << std::setw(sizeof(int))
        << std::hex << err;
    return stream.str();
}

static void GLClearError() {
    while (glGetError()) {}
}

static bool GLCheckErrorSimple() {
    bool t = true;
    while (GLenum err = glGetError()) {
        std::cout << "ERR: (OPENGL) with error code " << err << " (" << IntToErrCode(err) << ")" << std::endl;
        t = false;
    }
    return t;
}

static bool GLCheckErrorDetailed(const char* fun, const char* file, int line) {
    bool t = true;
    while (GLenum err = glGetError()) {
        std::cout << "ERR: (OPENGL) with error code " << err << " (" << IntToErrCode(err) << ") when calling function " << fun << " at line " << line << " in " << file << std::endl;
        t = false;
    }
    return t;
}

#if USE_DETAILED_ERRORS == 1
#define GLCheckError(x) GLCheckErrorDetailed(#x, __FILE__, __LINE__)
#else
#define GLCheckError(x) GLCheckErrorSimple()
#endif

#define LOG(x) std::cout << x << std::endl  /* NOLINT(bugprone-macro-parentheses) */
#define DEBUGFUN(x) (x)
#define CHECKFUNERR(x) GLClearError(); (x); GLCheckError(x)

#if FORCE_BREAKPOINTS == 1
#define ASSERT(x) if (!(x)) __debugbreak()
#else
#define ASSERT(x) (x)
#endif
#define GLCALL(x) GLClearError(); x; ASSERT(GLCheckError(x)) /* warning: this is not in a new scope. if this is in a loop/if statement, be sure to add braces */


#else
#define LOG(x)
#define DEBUGFUN(x)
#define GLClearError()
#define GLCheckError()
#define CHECKFUNERR(x) (x)
#define TRY(x) (x)
#define GLCALL(x) (x)

static void GLClearError() {}
static bool GLCheckErrorSimple() = true;
static bool GLCheckErrorDetailed(const char* fun, const char* file, int line) = true;
#endif