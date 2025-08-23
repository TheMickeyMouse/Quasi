#pragma once
#include "GLs/VertexElement.h"
#include "Utils/Math/Color.h"

namespace Quasi::Graphics {
    struct UIVertex {
        Math::fv2 Position;
        Math::fv2 TexCoord;
        Math::fColor Color;
        Math::fv4 STUV;
        u32 RenderPrim = 0;

        QuasiDefineVertex$(UIVertex, 3D, (Position, Position)(TexCoord)(Color)(STUV)(RenderPrim))
    };

    namespace UIRender {
        enum RenderPrimitive {
            PLAIN  = 0,
            CIRCLE = 1,
            QBEZ   = 2,
            CBEZ   = 3,
            INVERT = 0x8
        };

        enum RenderStyle {
            ROUND_CAP  = 0x0,
            SQUARE_CAP = 0x2,
            FLAT_CAP   = 0x1,
            CAP_MASK   = 0xF,

            ROUND_JOIN = 0x00,
            MITER_JOIN = 0x20,
            BEVEL_JOIN = 0x10,
            JOIN_MASK  = 0xF0,

            NO_FILL    = 0x100,
            NO_STROKE  = 0x200,
            NO_RENDER  = NO_FILL | NO_STROKE
        };
    }
}
