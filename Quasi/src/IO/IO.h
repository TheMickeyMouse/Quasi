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
        KeyboardType Keyboard { *this };
        MouseType Mouse { *this };
        TimeType Time;

        IO(Graphics::GraphicsDevice& gd);
        void SetUserPtr();
        static IO* GetIOPtr(GLFWwindow* win);

        void Update();
    };
}
