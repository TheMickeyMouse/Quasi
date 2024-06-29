#include "TimeType.h"

#include <GLFW/glfw3.h>

namespace Quasi::IO {
    void TimeType::Update() {
        ++currentFrame;
        const double newTime = glfwGetTime();

        dtIndex = (dtIndex + 1) % DELTATIME_HISTORY_NUM;
        timeFor2s -= deltaTime[dtIndex];
        deltaTime[dtIndex] = newTime - currentTime;
        timeFor2s += deltaTime[dtIndex];

        currentTime = newTime;
    }

    void TimeType::SetTime(double time) {
        currentTime = time;
        glfwSetTime(time);
    }

    double TimeType::Framerate() const { return (double)DELTATIME_HISTORY_NUM / timeFor2s; }
}
