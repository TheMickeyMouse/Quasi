#include "IndexBuffer.h"
#include "Debugging.h"

namespace Graphics {
    IndexBuffer::IndexBuffer(std::span<const uint> data) : GLObject({}), length(data.size()) {
        GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size_bytes(), data.data(), GL_STATIC_DRAW));
    }
}
