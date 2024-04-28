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
        stdu::ref<Graphics::GraphicsDevice> gdevice;
        KeyboardType Keyboard { nullptr };
        MouseType Mouse { nullptr };
        TimeType Time;

        IO() = default;
        IO(Graphics::GraphicsDevice& gd);
        void SetUserPtr();
        static IO* GetIOPtr(GLFWwindow* win);

        void Update();
    };
}
