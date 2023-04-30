#include "Mouse.h"

#include "GraphicsDevice.h"

namespace IO {
    void Mouse::Update() {
        GLFWwindow* win = Graphics::GraphicsDevice::GetMainWindow();
        prevMouseStates = mouseStates;
        mouseStates = 0;
        for (int i = 0; i <= LAST_MOUSE; ++i) {
            mouseStates |= glfwGetMouseButton(win, i) << i;
        }
    }

    Maths::Vec2d Mouse::GetMousePosPx() const {
        Maths::Vec2d pos;
        glfwGetCursorPos(Graphics::GraphicsDevice::GetMainWindow(), &pos.x, &pos.y);
        return pos;
    }

    Maths::Vec2d Mouse::GetMousePos() const {
        auto r01 = GetMousePosPx() / Graphics::GraphicsDevice::GetDeviceInstance().GetWindowSize(); // range 0 - 1
        return r01 * 2.0 - 1.0;
    }

    bool Mouse::IsInWindow() const {
        auto [mouseX,  mouseY ] = GetMousePosPx();
        auto [borderX, borderY] = Graphics::GraphicsDevice::GetDeviceInstance().GetWindowSize();
        return 0 <= mouseX && mouseX <= borderX &&
               0 <= mouseY && mouseY <= borderY;
    }

    int  Mouse::PressedState()         const { return mouseStates; }
    bool Mouse::LeftPressed()          const { return PressedState() & 1 << LEFT_MOUSE;   }
    bool Mouse::RightPressed()         const { return PressedState() & 1 << RIGHT_MOUSE;  }
    bool Mouse::MiddlePressed()        const { return PressedState() & 1 << MIDDLE_MOUSE; }
    bool Mouse::ButtonPressed(int btn) const { return PressedState() & 1 << btn;          }
    bool Mouse::AnyPressed()           const { return PressedState() != 0; }
    bool Mouse::NonePressed()          const { return PressedState() == 0; }
    
    int  Mouse::OnPressState()         const { return ~prevMouseStates & mouseStates; }
    bool Mouse::LeftOnPress()          const { return OnPressState() & 1 << LEFT_MOUSE;   }
    bool Mouse::RightOnPress()         const { return OnPressState() & 1 << RIGHT_MOUSE;  }
    bool Mouse::MiddleOnPress()        const { return OnPressState() & 1 << MIDDLE_MOUSE; }
    bool Mouse::ButtonOnPress(int btn) const { return OnPressState() & 1 << btn;          }
    bool Mouse::AnyOnPress()           const { return OnPressState() != 0; }
    bool Mouse::NoneOnPress()          const { return OnPressState() == 0; }
    
    int  Mouse::OnReleaseState()         const { return prevMouseStates & ~mouseStates; }
    bool Mouse::LeftOnRelease()          const { return OnReleaseState() & 1 << LEFT_MOUSE;   }
    bool Mouse::RightOnRelease()         const { return OnReleaseState() & 1 << RIGHT_MOUSE;  }
    bool Mouse::MiddleOnRelease()        const { return OnReleaseState() & 1 << MIDDLE_MOUSE; }
    bool Mouse::ButtonOnRelease(int btn) const { return OnReleaseState() & 1 << btn;          }
    bool Mouse::AnyOnRelease()           const { return OnReleaseState() != 0; }
    bool Mouse::NoneOnRelease()          const { return OnReleaseState() == 0; }
    
    bool Mouse::IsStandardMouseButton(int btn) { return LEFT_MOUSE <= btn && btn <= MIDDLE_MOUSE; }
    bool Mouse::IsValidMouseButton(int btn) { return LEFT_MOUSE <= btn && btn <= LAST_MOUSE; }

    const char* Mouse::MouseButtonToStr(int btn) {
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
