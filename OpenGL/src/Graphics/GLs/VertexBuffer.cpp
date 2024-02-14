#include "VertexBuffer.h"
#include "../Debugging.h"

namespace Graphics {
    VertexBuffer::VertexBuffer(stdu::cbyte_span data) : GLObject({}) {
        GLCALL(glBufferData(GL_ARRAY_BUFFER, data.size_bytes(), data.data(), GL_STATIC_DRAW));
    }
}