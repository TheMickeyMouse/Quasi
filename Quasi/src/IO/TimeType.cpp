#include "TimeType.h"

#include <GLFW/glfw3.h>

namespace Quasi::IO {
    void TimeType::Update() {
        ++currentFrame;
        const float newTime = (float)glfwGetTime();

        dtIndex = (dtIndex + 1) % DELTATIME_HISTORY_NUM;
        timeFor2s -= deltaTime[dtIndex];
        deltaTime[dtIndex] = newTime - currentTime;
        timeFor2s += deltaTime[dtIndex];

        currentTime = newTime;
    }

    void TimeType::SetTime(float time) {
        currentTime = time;
        glfwSetTime(time);
    }

    float TimeType::Framerate() const {
        return (float)DELTATIME_HISTORY_NUM / timeFor2s;
    }
}
