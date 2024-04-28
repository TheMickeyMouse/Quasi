#pragma once
#include "GLObject.h"

#include "Texture.h"

namespace Graphics {
    struct RenderBufferHandler : GLObjectHandler<RenderBufferHandler> {
        [[nodiscard]] glID Create() const;
        void Destroy(glID id) const;
        void Bind(glID id) const;
        void Unbind() const;
    };

    class RenderBuffer : public GLObject<RenderBufferHandler> {
    public:
        RenderBuffer() = default;
        RenderBuffer(TextureIFormat format, Maths::ivec2 size);

        friend class FrameBuffer;
        friend class GraphicsDevice;
    };
}
