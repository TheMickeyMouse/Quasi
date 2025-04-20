#pragma once
#include <queue>

#include "Math/Vector.h"

struct GLFWwindow;

namespace Quasi::Graphics {
    class GraphicsDevice;
}

namespace Quasi::IO {
    class IO;

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

        void Update();

        void OnGlfwMouseCallback(GLFWwindow* window, int mouse, int action, int mods);
        void OnGlfwScrollCallback(GLFWwindow* window, double xOff, double yOff);

        void Lock();
        void Hide();
        void Show(); // will also unlock the cursor

        Math::dVector2 GetMousePosPx() const;
        Math::dVector2 GetMousePos() const;
        bool IsInWindow() const;

        Math::dVector2 GetMouseScroll() const;
        Math::dVector2 GetMouseScrollDelta() const;

        int  PressedState()         const;
        bool LeftPressed()          const;
        bool RightPressed()         const;
        bool MiddlePressed()        const;
        bool ButtonPressed(int btn) const;
        bool AnyPressed()           const;
        bool NonePressed()          const;

        int  OnPressState()         const;
        bool LeftOnPress()          const;
        bool RightOnPress()         const;
        bool MiddleOnPress()        const;
        bool ButtonOnPress(int btn) const;
        bool AnyOnPress()           const;
        bool NoneOnPress()          const;

        int  OnReleaseState()         const;
        bool LeftOnRelease()          const;
        bool RightOnRelease()         const;
        bool MiddleOnRelease()        const;
        bool ButtonOnRelease(int btn) const;
        bool AnyOnRelease()           const;
        bool NoneOnRelease()          const;

        static bool IsStandardMouseButton(int btn);
        static bool IsValidMouseButton(int btn);
        static Str  MouseButtonToStr(int btn);
        
        private:
            GLFWwindow* inputWindow();
            const GLFWwindow* inputWindow() const;
    };
}
