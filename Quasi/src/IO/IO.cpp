#include <GLFW/glfw3.h>

#include "Mouse.h"
#include "Keyboard.h"
#include "TimeType.h"
#include "IO.h"

#include "../Graphics/GraphicsDevice.h"

namespace Quasi::IO {
    IO::IO(Graphics::GraphicsDevice& gd) : gdevice(gd) {
        SetUserPtr();

        glfwSetFramebufferSizeCallback(gd.GetWindow(), [] (GLFWwindow* window, int width, int height) {
            IO* ioInstance = GetIOPtr(window);
            ioInstance->gdevice->windowSize = { width, height };
        });
    }

    void IO::SetUserPtr() {
        glfwSetWindowUserPointer(gdevice->GetWindow(), this);
    }

    IO* IO::GetIOPtr(GLFWwindow* win) {
        return (IO*)glfwGetWindowUserPointer(win);
    }

    void IO::Update() {
        Mouse.Update();
        Keyboard.Update();
        Time.Update();
    }
}
