#pragma once
#include "IndexBuffer.h"
#include "Shader.h"

namespace Quasi::Graphics {
    class RenderData;
    class VertexArray;

    enum class BufferBit {
        COLOR   = 0x00004000,
        DEPTH   = 0x00000100,
        STENCIL = 0x00000400,
        ACCUM   = 0x00000200,
        ALL = COLOR | DEPTH | STENCIL,
    };

    enum class Capability {
        BLEND                 = 0xBE2,
        CLIP_DIST_0           = 0x3000, CLIP_DIST_1, CLIP_DIST_2, CLIP_DIST_3, CLIP_DIST_4, CLIP_DIST_5, CLIP_DIST_6, CLIP_DIST_7,
        COLOR_LOGIC_OP        = 0xBF2,
        CULL_FACE             = 0xB44,
        DEPTH_CLAMP           = 0x864F,
        DEPTH                 = 0xB71,
        DITHER                = 0xBD0,
        SRGB_FRAMEBUFFER      = 0x8DB9,
        SMOOTH_LINES          = 0xB20,
        MULTISAMPLE           = 0x809D,
        POLYGON_FILL_OFFSET   = 0x8037, POLYGON_LINE_OFFSET = 0x2A02, POLYGON_POINT_OFFSET = 0x2A01, SMOOTH_POLYGON = 0xB41,
        RESTART_PRIMITIVE     = 0x8F9D,
        SAMPLE_ALPHA_COVERAGE = 0x809E,
        SAMPLE_ALPHA_ONE      = 0x809F,
        SAMPLE_COVERAGE       = 0x80A0,
        SCISSOR               = 0xC11,
        STENCIL               = 0xB90,
        SEAMLESS_CUBE_MAP     = 0x884F,
        SHADER_POINT_SIZE     = 0x8642,
    };

    enum class CmpOperation {
        NEVER    = 0x0200,
        LESS     = 0x0201,
        EQUAL    = 0x0202,
        LEQUAL   = 0x0203,
        GREATER  = 0x0204,
        NOTEQUAL = 0x0205,
        GEQUAL   = 0x0206,
        ALWAYS   = 0x0207,
    };

    enum class StencilOperation {
        ZERO      = 0,
        KEEP      = 0x1E00,
        REPLACE   = 0x1E01,
        INCR      = 0x1E02,
        INCR_WRAP = 0x8507,
        DECR      = 0x1E03,
        DECR_WRAP = 0x8508,
        INVERT    = 0x150A
    };

    enum class BlendFactor {
        ZERO               = 0,
        ONE                = 1,
        SRC_COLOR          = 0x0300,
        INVERT_SRC_COLOR   = 0x0301,
        SRC_ALPHA          = 0x0302,
        INVERT_SRC_ALPHA   = 0x0303,
        DST_ALPHA          = 0x0304,
        INVERT_DST_ALPHA   = 0x0305,
        DST_COLOR          = 0x0306,
        INVERT_DST_COLOR   = 0x0307,
        SRC_ALPHA_SATURATE = 0x0308,
        CONST_COLOR        = 0x8001,
        INVERT_CONST_COLOR = 0x8002,
        CONST_ALPHA        = 0x8003,
        INVERT_CONST_ALPHA = 0x8004
    };

    enum class RenderMode {
        POINTS = 0x1B00, LINES = 0x1B01, FILL = 0x1B02
    };

    enum class BufferMode {
        NONE           = 0x0,
        FRONT_LEFT     = 0x0400,
        FRONT_RIGHT    = 0x0401,
        BACK_LEFT      = 0x0402,
        BACK_RIGHT     = 0x0403,
        FRONT          = 0x0404,
        BACK           = 0x0405,
        LEFT           = 0x0406,
        RIGHT          = 0x0407,
        FRONT_AND_BACK = 0x0408,
    };

    enum class FacingMode {
        BACK  = 0x0404,
        FRONT = 0x0405,
        FRONT_AND_BACK = 0x0408,
    };

    enum class OrientationMode {
        CLOCKWISE = 0x0900,
        COUNTER_CLOCKWISE = 0x0901,
    };
}

namespace Quasi::Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader);
    void DrawInstanced(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader, int instances);
    void Draw(const RenderData& dat, const Shader& s);
    void Draw(const RenderData& dat);
    void DrawInstanced(const RenderData& dat, const Shader& s, int instances);
    void DrawInstanced(const RenderData& dat, int instances);

#pragma region GL Functions
#define GL_SWITCH(F, NAME, E) inline void F##NAME() { F(E); }

#define GL_CLEAR_SWITCH(N, E) GL_SWITCH(Clear, N, BufferBit::E)
    void Clear(BufferBit bit = BufferBit::ALL);
    GL_CLEAR_SWITCH(ColorBit, COLOR)
    GL_CLEAR_SWITCH(DepthBit, DEPTH)
    GL_CLEAR_SWITCH(StencilBit, STENCIL)
#undef GL_CLEAR_SWITCH

    void SetClearColor(const Math::fColor& color);
    void SetRenderMode(RenderMode mode);
    void SetPointSize(float size);
    inline void SetRenderFill()   { SetRenderMode(RenderMode::FILL); }
    inline void SetRenderLines()  { SetRenderMode(RenderMode::LINES); }
    inline void SetRenderPoints() { SetRenderMode(RenderMode::POINTS); }

#define GL_CAP_SWITCH(N, E) GL_SWITCH(Enable, N, Capability::E) GL_SWITCH(Disable, N, Capability::E)
    void Enable(Capability cap);
    void Disable(Capability cap);
    GL_CAP_SWITCH(Blend, BLEND)
    GL_CAP_SWITCH(Depth, DEPTH)
    GL_CAP_SWITCH(Stencil, STENCIL)
    GL_CAP_SWITCH(CullFace, CULL_FACE)
    GL_CAP_SWITCH(Multisample, MULTISAMPLE)
#undef GL_CAP_SWITCH

    void UseDepthFunc(CmpOperation op);

    void UseStencilTest(CmpOperation op, int ref, int mask = 0xFF);
    void UseStencilWriteMask(int mask);
    inline void EnableStencilWrite()  { UseStencilWriteMask(0xFF); }
    inline void DisableStencilWrite() { UseStencilWriteMask(0x00); }
    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass);

    void UseAlphaFunc(CmpOperation op, float ref);

    void UseBlendConstColor(const Math::fColor& ref);
    void UseBlendFunc(BlendFactor src, BlendFactor dest);

    void SetCullFace(FacingMode facing);
    void SetFrontFacing(OrientationMode orientation);

    void SetColorWrite(BufferMode mode);

    void SetViewport(const Math::iRect2D& viewport);
#undef GL_SWITCH
#pragma endregion
}
