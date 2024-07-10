#include "VertexBuffer.h"

#include <GL/glew.h>

#include "GLDebug.h"

namespace Quasi::Graphics {
    VertexBuffer::VertexBuffer(u32 size) : GLObject(), bufferSize(size) {
        Create();
        Q_GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
    }
    
    void VertexBuffer::SetDataBytes(Span<const byte> data) {
        Bind();
        Q_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, (int)data.size_bytes(), data.data()));
    }

    void VertexBuffer::ClearData() {
        dataOffset = 0;
    }

    void VertexBuffer::AddDataBytes(Span<const byte> data) {
        Bind();
        Q_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, (int)dataOffset, (int)data.size_bytes(), data.data()));
        dataOffset += data.size_bytes();
    }
}
