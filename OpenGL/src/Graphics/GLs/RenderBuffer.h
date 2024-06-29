#pragma once
#include "GLObject.h"
#include "Textures/TextureConstants.h"
#include "Math/Vector.h"


namespace Quasi::Graphics {
    struct RenderBufferHandler {
        [[nodiscard]] GraphicsID Create() const;
        void Destroy(GraphicsID id) const;
        void Bind(GraphicsID id) const;
        void Unbind() const;
    };

    class RenderBuffer : public GLObject<RenderBufferHandler> {
    public:
        RenderBuffer() = default;
        RenderBuffer(TextureIFormat format, Math::iVector2 size);

        friend class FrameBuffer;
        friend class GraphicsDevice;
    };
}
