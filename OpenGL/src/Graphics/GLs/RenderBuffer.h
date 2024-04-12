#pragma once
#include "GLObject.h"
#include <core.h>
#include "Texture.h"

namespace Graphics {
    struct RenderBufferHandler : GLObjectHandler<RenderBufferHandler> {
        [[nodiscard]] OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };

    class RenderBuffer : public GLObject<RenderBufferHandler> {
    public:
        RenderBuffer() = default;
        OPENGL_API RenderBuffer(TextureIFormat format, Maths::ivec2 size);

        friend class FrameBuffer;
        friend class GraphicsDevice;
    };
}
