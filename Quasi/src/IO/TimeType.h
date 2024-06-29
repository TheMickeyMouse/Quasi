#pragma once
#include "Utils/Type.h"

namespace Quasi::IO {
    class TimeType;

    class TimeType {
        static constexpr u32 DELTATIME_HISTORY_NUM = 120; // 2 sec of data
        double deltaTime[DELTATIME_HISTORY_NUM] {};
        double timeFor2s = 0.0f;
        u32 dtIndex = DELTATIME_HISTORY_NUM - 1;
    public:
        u64 currentFrame = 0;
        double currentTime = 0;

        explicit TimeType() = default;

        void Update();
        void SetTime(double time);
        [[nodiscard]] double Framerate() const;

        [[nodiscard]] double DeltaTime() const { return deltaTime[dtIndex]; }
        [[nodiscard]] float DeltaTimef() const { return (float)DeltaTime(); }
        [[nodiscard]] float Frameratef() const { return (float)Framerate(); }
    };
}
