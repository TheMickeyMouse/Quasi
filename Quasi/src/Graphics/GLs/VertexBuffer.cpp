#include "VertexBuffer.h"

#include <GL/glew.h>

#include "GLDebug.h"

namespace Quasi::Graphics {
    VertexBuffer::VertexBuffer(GraphicsID id, u32 size) : GLObject(id), bufferSize(size) {}

    VertexBuffer VertexBuffer::New(u32 size) {
        GraphicsID id;
        Q_GL_CALL(glGenBuffers(1, &id));
        BindObject(id);
        Q_GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
        return VertexBuffer { id, size };
    }

    void VertexBuffer::DestroyObject(GraphicsID id) {
        Q_GL_CALL(glDeleteBuffers(1, &id));
    }

    void VertexBuffer::BindObject(GraphicsID id) {
        Q_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id));
    }

    void VertexBuffer::UnbindObject() {
        Q_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
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
