#pragma once
#include "NumTypes.h"


namespace IO {
    class TimeType;

    class TimeType {
    public:
        uint64 currentFrame = 0;
        double currentTime = 0;
        double deltaTime = 0;

        explicit TimeType() = default;

        void Update();
        void SetTime(double time);
        [[nodiscard]] double Framerate() const;

        [[nodiscard]] float DeltaTimef() const { return (float)deltaTime; }
        [[nodiscard]] float Frameratef() const { return (float)Framerate(); }
    };
}
