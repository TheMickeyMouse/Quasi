#include "VertexBuffer.h"

#include "GL/glew.h"
#include "GLDebug.h"

namespace Graphics {
    VertexBuffer::VertexBuffer(stdu::cbyte_span data) : GLObject({}) {
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, data.size_bytes(), data.data(), GL_STATIC_DRAW));
    }
}