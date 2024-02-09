#include "VertexBuffer.h"
#include "../Debugging.h"

namespace Graphics {
    VertexBuffer::VertexBuffer(const void* data, uint size) : GLObject({}) {
        GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    }
}