#pragma once
#include <queue>

#include "Math/Vector.h"

struct GLFWwindow;

namespace Quasi::Graphics {
    class GraphicsDevice;
}

namespace Quasi::IO {
    class IO;
    
    // TODO: USE INTERFERE INSTEAD OF POLLING WITH GLFWMOSUECALLBACK
    struct MouseType {
        static constexpr int LEFT_MOUSE   = 0;
        static constexpr int RIGHT_MOUSE  = 1;
        static constexpr int MIDDLE_MOUSE = 2;
        static constexpr int LAST_MOUSE   = 7; // GLFW_MOUSE_BUTTON_LAST

        int mouseStates = 0;
        int prevMouseStates = 0;
        std::queue<char> queuedMouseEvents;

        Math::dVector2 scroll, scrollDelta;
        std::queue<Math::dVector2> queuedScrolls;

        Ref<IO> io;

        explicit MouseType(IO& io);
        explicit MouseType(std::nullptr_t) {}

        void Update();

        void OnGlfwMouseCallback(GLFWwindow* window, int mouse, int action, int mods);
        void OnGlfwScrollCallback(GLFWwindow* window, double xOff, double yOff);

        void Lock();
        void Hide();
        void Show(); // will also unlock the cursor

        [[nodiscard]] Math::dVector2 GetMousePosPx() const;
        [[nodiscard]] Math::dVector2 GetMousePos() const;
        [[nodiscard]] bool IsInWindow() const;

        [[nodiscard]] Math::dVector2 GetMouseScroll() const;
        [[nodiscard]] Math::dVector2 GetMouseScrollDelta() const;

        [[nodiscard]] int  PressedState()         const;
        [[nodiscard]] bool LeftPressed()          const;
        [[nodiscard]] bool RightPressed()         const;
        [[nodiscard]] bool MiddlePressed()        const;
        [[nodiscard]] bool ButtonPressed(int btn) const;
        [[nodiscard]] bool AnyPressed()           const;
        [[nodiscard]] bool NonePressed()          const;

        [[nodiscard]] int  OnPressState()         const;
        [[nodiscard]] bool LeftOnPress()          const;
        [[nodiscard]] bool RightOnPress()         const;
        [[nodiscard]] bool MiddleOnPress()        const;
        [[nodiscard]] bool ButtonOnPress(int btn) const;
        [[nodiscard]] bool AnyOnPress()           const;
        [[nodiscard]] bool NoneOnPress()          const;

        [[nodiscard]] int  OnReleaseState()         const;
        [[nodiscard]] bool LeftOnRelease()          const;
        [[nodiscard]] bool RightOnRelease()         const;
        [[nodiscard]] bool MiddleOnRelease()        const;
        [[nodiscard]] bool ButtonOnRelease(int btn) const;
        [[nodiscard]] bool AnyOnRelease()           const;
        [[nodiscard]] bool NoneOnRelease()          const;

        static bool IsStandardMouseButton(int btn);
        static bool IsValidMouseButton(int btn);
        static Str  MouseButtonToStr(int btn);
        
        private:
            GLFWwindow* inputWindow();
            [[nodiscard]] const GLFWwindow* inputWindow() const;
    };
}
