#pragma once
#include "NumTypes.h"
#include "opengl.h"
#include "Texture.h"

namespace Graphics {
    class RenderBuffer {
    private:
        glID rendererID = GL_NULL;
    public:
        RenderBuffer() = default;
        OPENGL_API RenderBuffer(TextureInternalFormat format, Maths::ivec2 size);
        OPENGL_API ~RenderBuffer();

        RenderBuffer(const RenderBuffer&) = delete;
        RenderBuffer& operator=(const RenderBuffer&) = delete;
        OPENGL_API static void Transfer(RenderBuffer& dest, RenderBuffer&& from);
        RenderBuffer(RenderBuffer&& rbo) noexcept { Transfer(*this, std::move(rbo)); }
        RenderBuffer& operator=(RenderBuffer&& fbo) noexcept { Transfer(*this, std::move(fbo)); return *this; }

        OPENGL_API void Bind() const;
        OPENGL_API void Unbind() const;

        friend class FrameBuffer;
    };
}
