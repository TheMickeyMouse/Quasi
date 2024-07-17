#pragma once
#include "GLObject.h"
#include "Textures/TextureConstants.h"
#include "Math/Vector.h"

namespace Quasi::Graphics {
    class RenderBuffer : public GLObject<RenderBuffer> {
        explicit RenderBuffer(GraphicsID id);
    public:
        RenderBuffer() = default;
        static RenderBuffer New(TextureIFormat format, Math::iVector2 size);
        static void DestroyObject(GraphicsID id);
        static void BindObject(GraphicsID id);
        static void UnbindObject();

        friend class FrameBuffer;
        friend class GraphicsDevice;
    };
}
