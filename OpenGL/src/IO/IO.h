#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "TimeType.h"
// ! THIS INCLUDES GLEW.H !

namespace Graphics {
    class GraphicsDevice;
}

namespace IO {
    class IO {
    public:
        KeyboardType Keyboard { nullptr };
        MouseType Mouse { nullptr };
        TimeType Time;

        OPENGL_API IO() = default;
        OPENGL_API IO(Graphics::GraphicsDevice& gd);
        OPENGL_API void Update();
    };
}
