#include "Mouse.h"
#include "Keyboard.h"
#include "TimeType.h"
#include "IO.h"

#include "GraphicsDevice.h"

namespace IO {
    IO::IO(Graphics::GraphicsDevice& gd) :
        gdevice(gd),
        Keyboard(*this),
        Mouse(*this) {
        SetUserPtr();
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
