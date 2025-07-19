#pragma once
#include "GLs/VertexElement.h"
#include "Utils/Math/Color.h"
#include "Utils/Math/Vector.h"

namespace Quasi {
    struct UIVertex {
        Math::fv3 Position;
        u32 RenderIndex;
        Math::fColor Color;
        Math::fv2 Texture;
        Math::fv2 UV;

        QuasiDefineVertex$(UIVertex, 2D, (Position, Position)(RenderIndex)(Color)(Texture)(UV));
    };

    class Layer {

    };
} // Quasi
