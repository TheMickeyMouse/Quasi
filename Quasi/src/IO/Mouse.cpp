#include "../Graphics/GraphicsDevice.h"
#include <GLFW/glfw3.h>

#include "IO.h"

#include "imgui_impl_glfw.h"

namespace Quasi::IO {
    GLFWwindow* MouseType::inputWindow() { return io->gdevice->GetWindow(); }
    const GLFWwindow* MouseType::inputWindow() const { return io->gdevice->GetWindow(); }

    MouseType::MouseType(IO& io) : io(io) {
        glfwSetMouseButtonCallback(inputWindow(),
            [](GLFWwindow* window, int button, int action, int mods) {
                IO::GetIOPtr(window)->Mouse.OnGlfwMouseCallback(window, button, action, mods);
            });

        glfwSetScrollCallback(inputWindow(),
            [](GLFWwindow* window, double xOff, double yOff) {
                IO::GetIOPtr(window)->Mouse.OnGlfwScrollCallback(window, xOff, yOff);
            });
    }

    void MouseType::Update() {
        prevMouseStates = mouseStates;
        while (!queuedMouseEvents.empty()) {
            const char event = queuedMouseEvents.front();
            const bool state = event >= 0;
            const int btn = state ? event : ~event;
            mouseStates = (mouseStates & ~(1 << btn)) | (state << btn);
            queuedMouseEvents.pop();
        }

        scrollDelta = 0;
        while (!queuedScrolls.empty()) {
            const Math::dv2 scrollEvent = queuedScrolls.front();
            scrollDelta += scrollEvent;
            scroll += scrollEvent;
            queuedScrolls.pop();
        }
    }

    void MouseType::OnGlfwMouseCallback(GLFWwindow* window, int mouse, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, mouse, action, mods);
        queuedMouseEvents.emplace(action == GLFW_PRESS ? mouse : ~mouse);
    }

    void MouseType::OnGlfwScrollCallback(GLFWwindow* window, double xOff, double yOff) {
        ImGui_ImplGlfw_ScrollCallback(window, xOff, yOff);
        queuedScrolls.emplace(xOff, yOff);
    }

    void MouseType::Lock() {
        glfwSetInputMode(inputWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void MouseType::Hide() {
        glfwSetInputMode(inputWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void MouseType::Show() {
        glfwSetInputMode(inputWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    Math::dv2 MouseType::GetMousePosPx() const {
        Math::dv2 pos;
        glfwGetCursorPos((GLFWwindow*)inputWindow(), &pos.x, &pos.y);
        return pos;
    }

    Math::dv2 MouseType::GetMousePos() const {
        const auto r01 = GetMousePosPx() / (Math::dv2)io->gdevice->GetWindowSize(); // range 0 - 1
        return r01 * 2.0 - 1.0;
    }

    bool MouseType::IsInWindow() const {
        auto [mouseX,  mouseY ] = GetMousePosPx();
        auto [borderX, borderY] = io->gdevice->GetWindowSize();
        return 0 <= mouseX && mouseX <= borderX &&
               0 <= mouseY && mouseY <= borderY;
    }

    Math::dv2 MouseType::GetMouseScroll() const {
        return scroll;
    }

    Math::dv2 MouseType::GetMouseScrollDelta() const {
        return scrollDelta;
    }

    int  MouseType::PressedState()         const { return mouseStates; }
    bool MouseType::LeftPressed()          const { return PressedState() & 1 << LEFT_MOUSE;   }
    bool MouseType::RightPressed()         const { return PressedState() & 1 << RIGHT_MOUSE;  }
    bool MouseType::MiddlePressed()        const { return PressedState() & 1 << MIDDLE_MOUSE; }
    bool MouseType::ButtonPressed(int btn) const { return PressedState() & 1 << btn;          }
    bool MouseType::AnyPressed()           const { return PressedState() != 0; }
    bool MouseType::NonePressed()          const { return PressedState() == 0; }
    
    int  MouseType::OnPressState()         const { return ~prevMouseStates & mouseStates; }
    bool MouseType::LeftOnPress()          const { return OnPressState() & 1 << LEFT_MOUSE;   }
    bool MouseType::RightOnPress()         const { return OnPressState() & 1 << RIGHT_MOUSE;  }
    bool MouseType::MiddleOnPress()        const { return OnPressState() & 1 << MIDDLE_MOUSE; }
    bool MouseType::ButtonOnPress(int btn) const { return OnPressState() & 1 << btn;          }
    bool MouseType::AnyOnPress()           const { return OnPressState() != 0; }
    bool MouseType::NoneOnPress()          const { return OnPressState() == 0; }
    
    int  MouseType::OnReleaseState()         const { return prevMouseStates & ~mouseStates; }
    bool MouseType::LeftOnRelease()          const { return OnReleaseState() & 1 << LEFT_MOUSE;   }
    bool MouseType::RightOnRelease()         const { return OnReleaseState() & 1 << RIGHT_MOUSE;  }
    bool MouseType::MiddleOnRelease()        const { return OnReleaseState() & 1 << MIDDLE_MOUSE; }
    bool MouseType::ButtonOnRelease(int btn) const { return OnReleaseState() & 1 << btn;          }
    bool MouseType::AnyOnRelease()           const { return OnReleaseState() != 0; }
    bool MouseType::NoneOnRelease()          const { return OnReleaseState() == 0; }
    
    bool MouseType::IsStandardMouseButton(int btn) { return LEFT_MOUSE <= btn && btn <= MIDDLE_MOUSE; }
    bool MouseType::IsValidMouseButton(int btn) { return LEFT_MOUSE <= btn && btn <= LAST_MOUSE; }

    Str MouseType::MouseButtonToStr(int btn) {
        switch (btn) {
            case LEFT_MOUSE:   return "Left Mouse Button";
            case RIGHT_MOUSE:  return "Right Mouse Button";
            case MIDDLE_MOUSE: return "Middle Mouse Button";

            // not a great method but i want to use const char*
            case 3: return "No. 3 Mouse Button";
            case 4: return "No. 4 Mouse Button";
            case 5: return "No. 5 Mouse Button";
            case 6: return "No. 6 Mouse Button";
            case 7: return "No. 7 Mouse Button";
            default: return "";
        }
    }
}
