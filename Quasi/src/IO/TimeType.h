#pragma once
#include "Utils/Type.h"

namespace Quasi::IO {
    class TimeType;

    class TimeType {
        static constexpr u32 DELTATIME_HISTORY_NUM = 120; // 2 sec of data
        float deltaTime[DELTATIME_HISTORY_NUM] {};
        float timeFor2s = 0.0f;
        u32 dtIndex = DELTATIME_HISTORY_NUM - 1;
    public:
        u64 currentFrame = 0;
        float currentTime = 0;

        explicit TimeType() = default;

        void Update();
        void SetTime(float time);

        [[nodiscard]] float DeltaTime() const { return deltaTime[dtIndex]; }
        [[nodiscard]] float Framerate() const;
    };
}
