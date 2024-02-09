#pragma once
#include "GLObject.h"
#include "NumTypes.h"
#include "opengl.h"
#include "Texture.h"

namespace Graphics {
    struct RenderBufferHandler : GLObjectHandler<RenderBufferHandler> {
        OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };

    class RenderBuffer : public GLObject<RenderBufferHandler> {
    public:
        RenderBuffer() = default;
        OPENGL_API RenderBuffer(TextureInternalFormat format, Maths::ivec2 size);

        friend class FrameBuffer;
    };
}
