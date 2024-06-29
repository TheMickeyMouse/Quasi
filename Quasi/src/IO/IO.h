#pragma once
#include "Keyboard.h"
#include "Mouse.h"
#include "TimeType.h"
// ! THIS INCLUDES GLEW.H !

namespace Quasi::Graphics {
    class GraphicsDevice;
}

namespace Quasi::IO {
    class IO {
    public:
        Ref<Graphics::GraphicsDevice> gdevice;
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
