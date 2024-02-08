#include "Time.h"

#include <GLFW/glfw3.h>

namespace IO {
    OPENGL_API TimeType Time {};

    void TimeType::Update() {
        ++currentFrame;
        const double newTime = glfwGetTime();
        deltaTime = newTime - currentTime;
        currentTime = newTime;
    }

    void TimeType::SetTime(double time) {
        currentTime = time;
        glfwSetTime(time);
    }

    double TimeType::Framerate() const { return 1.0 / deltaTime; }
}
