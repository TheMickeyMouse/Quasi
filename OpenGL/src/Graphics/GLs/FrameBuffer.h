#pragma once
#include <algorithm>

#include "NumTypes.h"
#include "opengl.h"

namespace Graphics {
    class RenderBuffer;
    class Texture;

    enum class AttachmentType {
        COLOR_0  = 0x8CE0,
        COLOR_1  = 0x8CE1,
        COLOR_2  = 0x8CE2,
        COLOR_3  = 0x8CE3,
        COLOR_4  = 0x8CE4,
        COLOR_5  = 0x8CE5,
        COLOR_6  = 0x8CE6,
        COLOR_7  = 0x8CE7,
        COLOR_8  = 0x8CE8,
        COLOR_9  = 0x8CE9,
        COLOR_10 = 0x8CEA,
        COLOR_11 = 0x8CEB,
        COLOR_12 = 0x8CEC,
        COLOR_13 = 0x8CED,
        COLOR_14 = 0x8CEE,
        COLOR_15 = 0x8CEF,
        DEPTH = 0x8D00,
        STENCIL = 0x8D20,
        DEPTH_STENCIL = 0x821A,
    };

    class FrameBuffer {
    private:
        glID rendererID = GL_NULL;
    public:
        FrameBuffer() = default;
        OPENGL_API FrameBuffer(int);
        OPENGL_API ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;
        OPENGL_API static void Transfer(FrameBuffer& dest, FrameBuffer&& from);
        FrameBuffer(FrameBuffer&& fbo) noexcept { Transfer(*this, std::move(fbo)); }
        FrameBuffer& operator=(FrameBuffer&& fbo) noexcept { Transfer(*this, std::move(fbo)); return *this; }

        OPENGL_API void Bind() const;
        OPENGL_API void Unbind() const;

        OPENGL_API void Attach(const Texture& tex) const;
        OPENGL_API void Attach(const RenderBuffer& rbo, AttachmentType type) const;
        OPENGL_API void Complete() const;
    };
}
