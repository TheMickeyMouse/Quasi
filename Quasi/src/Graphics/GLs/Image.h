#pragma once

#include "Utils/Box.h"
#include "Utils/Math/Vector.h"

namespace Quasi::Graphics {
    struct Image {
        Box<u8> imageData;
        u32 width, height;

        Math::uv2 Size() const { return { width, height }; }
    };
}
