#pragma once
#include "Mouse.h"

namespace IO {
    inline extern Mouse Mouse = {};

    inline void Update() {
        Mouse.Update();
    }
}
