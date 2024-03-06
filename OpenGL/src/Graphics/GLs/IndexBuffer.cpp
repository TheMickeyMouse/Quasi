#include "IndexBuffer.h"
#include <GL/glew.h>
#include "GLDebug.h"

namespace Graphics {
    IndexBuffer::IndexBuffer(std::span<const uint> data) : GLObject({}), length(data.size()) {
        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size_bytes(), data.data(), GL_STATIC_DRAW));
    }
}
