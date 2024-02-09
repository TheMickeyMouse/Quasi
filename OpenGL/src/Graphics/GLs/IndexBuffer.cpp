#include "IndexBuffer.h"
#include "Debugging.h"

namespace Graphics {
    IndexBuffer::IndexBuffer(const void* data, uint length) : GLObject({}), length(length) {
        GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(uint), data, GL_STATIC_DRAW));
    }
}
