#pragma once
#include "IndexBuffer.h"
#include "Shader.h"
#include "RenderObject.h"

namespace Graphics {
    enum class BufferBit {
        COLOR = 0x00004000, DEPTH = 0x00000100, STENCIL = 0x00000400, ACCUM = 0x00000200,
        ALL = COLOR | DEPTH | STENCIL,
    };

    enum class Capability {
        BLEND = 0xBE2,
        CLIP_DIST_0 = 0x3000, CLIP_DIST_1, CLIP_DIST_2, CLIP_DIST_3, CLIP_DIST_4, CLIP_DIST_5, CLIP_DIST_6, CLIP_DIST_7,
        COLOR_LOGIC_OP = 0xBF2,
        CULL_FACE = 0xB44,
        DEPTH_CLAMP = 0x864F,
        DEPTH = 0xB71,
        DITHER = 0xBD0,
        SRGB_FRAMEBUFFER = 0x8DB9,
        SMOOTH_LINES = 0xB20,
        MULTISAMPLE = 0x809D,
        POLYGON_FILL_OFFSET = 0x8037, POLYGON_LINE_OFFSET = 0x2A02, POLYGON_POINT_OFFSET = 0x2A01, SMOOTH_POLYGON = 0xB41,
        RESTART_PRIMITIVE = 0x8F9D,
        SAMPLE_ALPHA_COVERAGE = 0x809E,
        SAMPLE_ALPHA_ONE = 0x809F,
        SAMPLE_COVERAGE = 0x80A0,
        SCISSOR = 0xC11,
        STENCIL = 0xB90,
        SEAMLESS_CUBE_MAP = 0x884F,
        SHADER_POINT_SIZE = 0x8642,
    };

    enum class CmpOperation {
        NEVER = 0x0200,
        LESS  = 0x0201,
        EQUAL = 0x0202,
        LEQUAL = 0x0203,
        GREATER = 0x0204,
        NOTEQUAL = 0x0205,
        GEQUAL = 0x0206,
        ALWAYS = 0x0207,
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
}

namespace Graphics::Render {
    OPENGL_API void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader);
    OPENGL_API void Draw(const VertexArray& vertexArr, const DynamicIndexBuffer& indexBuff, const Shader& shader);
    inline void Draw(const RenderData& dat) { Draw(dat.GetVertArr(), dat.GetIndObj(), dat.GetShader()); }
    inline void Draw(const RenderData& dat, const Shader& s) { Draw(dat.GetVertArr(), dat.GetIndObj(), s); }

#pragma region GL Functions
#define GL_SWITCH(F, NAME, E) inline void F##NAME() { F(E); }

#define GL_CLEAR_SWITCH(N, E) GL_SWITCH(Clear, N, BufferBit::E)
    OPENGL_API void Clear(BufferBit bit = BufferBit::ALL);
    GL_CLEAR_SWITCH(ColorBit, COLOR)
    GL_CLEAR_SWITCH(DepthBit, DEPTH)
    GL_CLEAR_SWITCH(StencilBit, STENCIL)
#undef GL_CLEAR_SWITCH

    OPENGL_API void SetClearColor(const Maths::colorf& color);
    OPENGL_API void SetRenderWireframe(bool isWireframe);

#define GL_CAP_SWITCH(N, E) GL_SWITCH(Enable, N, Capability::E) GL_SWITCH(Disable, N, Capability::E)
    OPENGL_API void Enable(Capability cap);
    OPENGL_API void Disable(Capability cap);
    GL_CAP_SWITCH(Blend, BLEND)
    GL_CAP_SWITCH(Depth, DEPTH)
    GL_CAP_SWITCH(Stencil, STENCIL)
    GL_CAP_SWITCH(CullFace, CULL_FACE)
    GL_CAP_SWITCH(Multisample, MULTISAMPLE)
#undef GL_CAP_SWITCH

    OPENGL_API void UseDepthFunc(CmpOperation op);

    OPENGL_API void UseStencilTest(CmpOperation op, int ref, int mask = 0xFF);
    OPENGL_API void UseStencilWriteMask(int mask);
    inline void EnableStencilWrite()  { UseStencilWriteMask(0xFF); }
    inline void DisableStencilWrite() { UseStencilWriteMask(0x00); }
    OPENGL_API void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass);

    OPENGL_API void UseAlphaFunc(CmpOperation op, float ref);

    OPENGL_API void UseBlendConstColor(const Maths::colorf& ref);
    OPENGL_API void UseBlendFunc(BlendFactor src, BlendFactor dest);

#undef GL_SWITCH
#pragma endregion
}
