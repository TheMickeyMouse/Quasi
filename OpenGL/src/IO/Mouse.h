#pragma once
#include <queue>

#include "opengl.h"

#include "Vector.h"
#include "stdu/ref.h"

struct GLFWwindow;

namespace Graphics {
    class GraphicsDevice;
}

namespace IO {
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

        Maths::dvec2 scroll, scrollDelta;
        std::queue<Maths::dvec2> queuedScrolls;

        stdu::ref<IO> io;

        OPENGL_API explicit MouseType(IO& io);
        explicit MouseType(std::nullptr_t) {}

        OPENGL_API void Update();

        OPENGL_API void OnGlfwMouseCallback(GLFWwindow* window, int mouse, int action, int mods);
        OPENGL_API void OnGlfwScrollCallback(GLFWwindow* window, double xOff, double yOff);

        OPENGL_API void Lock();
        OPENGL_API void Hide();
        OPENGL_API void Show(); // will also unlock the cursor

        OPENGL_API [[nodiscard]] Maths::dvec2 GetMousePosPx();
        OPENGL_API [[nodiscard]] Maths::dvec2 GetMousePos();
        OPENGL_API [[nodiscard]] bool IsInWindow();

        OPENGL_API [[nodiscard]] Maths::dvec2 GetMouseScroll() const;
        OPENGL_API [[nodiscard]] Maths::dvec2 GetMouseScrollDelta() const;

        OPENGL_API [[nodiscard]] int  PressedState()         const;
        OPENGL_API [[nodiscard]] bool LeftPressed()          const;
        OPENGL_API [[nodiscard]] bool RightPressed()         const;
        OPENGL_API [[nodiscard]] bool MiddlePressed()        const;
        OPENGL_API [[nodiscard]] bool ButtonPressed(int btn) const;
        OPENGL_API [[nodiscard]] bool AnyPressed()           const;
        OPENGL_API [[nodiscard]] bool NonePressed()          const;

        OPENGL_API [[nodiscard]] int  OnPressState()         const;
        OPENGL_API [[nodiscard]] bool LeftOnPress()          const;
        OPENGL_API [[nodiscard]] bool RightOnPress()         const;
        OPENGL_API [[nodiscard]] bool MiddleOnPress()        const;
        OPENGL_API [[nodiscard]] bool ButtonOnPress(int btn) const;
        OPENGL_API [[nodiscard]] bool AnyOnPress()           const;
        OPENGL_API [[nodiscard]] bool NoneOnPress()          const;

        OPENGL_API [[nodiscard]] int  OnReleaseState()         const;
        OPENGL_API [[nodiscard]] bool LeftOnRelease()          const;
        OPENGL_API [[nodiscard]] bool RightOnRelease()         const;
        OPENGL_API [[nodiscard]] bool MiddleOnRelease()        const;
        OPENGL_API [[nodiscard]] bool ButtonOnRelease(int btn) const;
        OPENGL_API [[nodiscard]] bool AnyOnRelease()           const;
        OPENGL_API [[nodiscard]] bool NoneOnRelease()          const;

        OPENGL_API static bool IsStandardMouseButton(int btn);
        OPENGL_API static bool IsValidMouseButton(int btn);
        OPENGL_API static const char* MouseButtonToStr(int btn);
        
        private:
            OPENGL_API GLFWwindow* inputWindow();
            OPENGL_API [[nodiscard]] const GLFWwindow* inputWindow() const;
    };
}
