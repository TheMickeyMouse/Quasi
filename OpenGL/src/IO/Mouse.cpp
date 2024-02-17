#include "GraphicsDevice.h"

#include "Mouse.h"

namespace IO {
    GLFWwindow* MouseType::inputWindow() { return graphicsDevice->GetWindow(); }
    const GLFWwindow* MouseType::inputWindow() const { return graphicsDevice->GetWindow(); }

    MouseType::MouseType(Graphics::GraphicsDevice& gd) : graphicsDevice(gd) {}

    void MouseType::Update() {
        prevMouseStates = mouseStates;
        mouseStates = 0;
        for (int i = 0; i <= LAST_MOUSE; ++i) {
            mouseStates |= glfwGetMouseButton(inputWindow(), i) << i;
        }
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

    Maths::dvec2 MouseType::GetMousePosPx() {
        Maths::dvec2 pos;
        glfwGetCursorPos(inputWindow(), &pos.x, &pos.y);
        return pos;
    }

    Maths::dvec2 MouseType::GetMousePos() {
        auto r01 = GetMousePosPx() / graphicsDevice->GetWindowSize(); // range 0 - 1
        return r01 * 2.0 - 1.0;
    }

    bool MouseType::IsInWindow() {
        auto [mouseX,  mouseY ] = GetMousePosPx();
        auto [borderX, borderY] = graphicsDevice->GetWindowSize();
        return 0 <= mouseX && mouseX <= borderX &&
               0 <= mouseY && mouseY <= borderY;
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

    const char* MouseType::MouseButtonToStr(int btn) {
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
            default: return nullptr;
        }
    }
}
