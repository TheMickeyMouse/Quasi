#pragma once
#include <chrono>

#include "Utils/Ref.h"
#include "Utils/Str.h"

namespace Quasi::Debug {
    class Logger;

    using DateTime = std::chrono::time_point<std::chrono::system_clock>;
    using TimeDuration = std::chrono::duration<u64, std::ratio<1, 1'000'000'000>>;

    using Second      = std::chrono::duration<u64>;
    using Millisecond = std::chrono::duration<u64, std::ratio<1, 1'000>>;
    using Microsecond = std::chrono::duration<u64, std::ratio<1, 1'000'000>>;
    using Nanosecond  = std::chrono::duration<u64, std::ratio<1, 1'000'000'000>>;

    class Timer {
    public:
        DateTime begin;
        TimeDuration total;
        bool paused = false;
        Str name;
        OptRef<Logger> outputLog;

        Timer(Str name = {}, OptRef<Logger> log = nullptr) : begin(Now()), total(0), name(name), outputLog(log) {}
        ~Timer();

        static DateTime Now();
        static TimeDuration Instant();

        template <class Unit> static u64 UnitConvert(TimeDuration d) {
            return std::chrono::duration_cast<Unit>(d).count();
        }

        void Start();
        TimeDuration Stop();
        void Resume() { return Start(); }
        void Reset();

        TimeDuration TotalElapsed() const;
        template <class Unit> u64 TotalElapsedUnits() const {
            return UnitConvert<Unit>(TotalElapsed());
        }
        u64 ElapsedMillis() const { return TotalElapsedUnits<Millisecond>(); }
        u64 ElapsedMicros() const { return TotalElapsedUnits<Microsecond>(); }
        u64 ElapsedNanos()  const { return TotalElapsedUnits<Nanosecond>(); }

        void Log();
    };
} // Quasi
