#pragma once
#include <queue>

#include <core.h>

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

        [[nodiscard]] OPENGL_API Maths::dvec2 GetMousePosPx();
        [[nodiscard]] OPENGL_API Maths::dvec2 GetMousePos();
        [[nodiscard]] OPENGL_API bool IsInWindow();

        [[nodiscard]] OPENGL_API Maths::dvec2 GetMouseScroll() const;
        [[nodiscard]] OPENGL_API Maths::dvec2 GetMouseScrollDelta() const;

        [[nodiscard]] OPENGL_API int  PressedState()         const;
        [[nodiscard]] OPENGL_API bool LeftPressed()          const;
        [[nodiscard]] OPENGL_API bool RightPressed()         const;
        [[nodiscard]] OPENGL_API bool MiddlePressed()        const;
        [[nodiscard]] OPENGL_API bool ButtonPressed(int btn) const;
        [[nodiscard]] OPENGL_API bool AnyPressed()           const;
        [[nodiscard]] OPENGL_API bool NonePressed()          const;

        [[nodiscard]] OPENGL_API int  OnPressState()         const;
        [[nodiscard]] OPENGL_API bool LeftOnPress()          const;
        [[nodiscard]] OPENGL_API bool RightOnPress()         const;
        [[nodiscard]] OPENGL_API bool MiddleOnPress()        const;
        [[nodiscard]] OPENGL_API bool ButtonOnPress(int btn) const;
        [[nodiscard]] OPENGL_API bool AnyOnPress()           const;
        [[nodiscard]] OPENGL_API bool NoneOnPress()          const;

        [[nodiscard]] OPENGL_API int  OnReleaseState()         const;
        [[nodiscard]] OPENGL_API bool LeftOnRelease()          const;
        [[nodiscard]] OPENGL_API bool RightOnRelease()         const;
        [[nodiscard]] OPENGL_API bool MiddleOnRelease()        const;
        [[nodiscard]] OPENGL_API bool ButtonOnRelease(int btn) const;
        [[nodiscard]] OPENGL_API bool AnyOnRelease()           const;
        [[nodiscard]] OPENGL_API bool NoneOnRelease()          const;

        OPENGL_API static bool IsStandardMouseButton(int btn);
        OPENGL_API static bool IsValidMouseButton(int btn);
        OPENGL_API static const char* MouseButtonToStr(int btn);
        
        private:
            OPENGL_API GLFWwindow* inputWindow();
            [[nodiscard]] OPENGL_API const GLFWwindow* inputWindow() const;
    };
}
