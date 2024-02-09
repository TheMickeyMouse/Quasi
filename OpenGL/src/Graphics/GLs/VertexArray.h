#pragma once
#include "DynamicVertexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

namespace Graphics {
    struct VertexArrayHandler : GLObjectHandler<VertexArrayHandler> {
        glID Create() const;
        void Destroy(glID id) const;
        void Bind(glID id) const;
        void Unbind() const;
    };

    class VertexArray : public GLObject<VertexArrayHandler> {
    public:
        VertexArray() = default;
        VertexArray(stdu::empty) : GLObject({}) {}

        OPENGL_API void AddBuffer(const VertexBufferLayout& layout);
        OPENGL_API void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
        OPENGL_API void AddBuffer(const DynamicVertexBuffer& vb, const VertexBufferLayout& layout);
    };
}
