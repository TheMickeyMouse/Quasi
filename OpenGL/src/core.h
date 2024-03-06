#pragma once

#ifdef OPENGL_EXPORTS
    #define OPENGL_API __declspec(dllexport)
#else
    #define OPENGL_API __declspec(dllimport)
#endif