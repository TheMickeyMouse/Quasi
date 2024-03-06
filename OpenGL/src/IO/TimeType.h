#pragma once
#include "NumTypes.h"
#include <core.h>

namespace IO {
    class TimeType;

    class TimeType {
    public:
        uint64 currentFrame = 0;
        double currentTime = 0;
        double deltaTime = 0;

        explicit TimeType() = default;

        OPENGL_API void Update();
        OPENGL_API void SetTime(double time);
        OPENGL_API [[nodiscard]] double Framerate() const;

        [[nodiscard]] float DeltaTimef() const { return (float)deltaTime; }
        [[nodiscard]] float Frameratef() const { return (float)Framerate(); }
    };
}
