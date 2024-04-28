#pragma once
#include "DynamicVertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

namespace Graphics {
    struct VertexArrayHandler : GLObjectHandler<VertexArrayHandler> {
        [[nodiscard]] glID Create() const;
        void Destroy(glID id) const;
        void Bind(glID id) const;
        void Unbind() const;
    };

    class VertexArray : public GLObject<VertexArrayHandler> {
    public:
        VertexArray() = default;
        VertexArray(stdu::empty) : GLObject({}) {}

        void AddBuffer(const VertexBufferLayout& layout);
        void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
        void AddBuffer(const DynamicVertexBuffer& vb, const VertexBufferLayout& layout);

        friend class GraphicsDevice;
    };
}
