#pragma once
#include "VertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

namespace Quasi::Graphics {
    struct VertexArrayHandler {
        [[nodiscard]] GraphicsID Create() const;
        void Destroy(GraphicsID id) const;
        void Bind(GraphicsID id) const;
        void Unbind() const;
    };

    class VertexArray : public GLObject<VertexArrayHandler> {
    public:
        VertexArray() = default;

        void AddBuffer(const VertexBufferLayout& layout);
        void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

        friend class GraphicsDevice;
    };
}
