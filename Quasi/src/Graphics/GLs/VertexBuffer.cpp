#include "VertexBuffer.h"

#include <glp.h>

#include "GLDebug.h"

namespace Quasi::Graphics {
    VertexBuffer::VertexBuffer(GraphicsID id, u32 size) : GLObject(id), bufferSize(size) {}

    VertexBuffer VertexBuffer::New(u32 size) {
        GraphicsID id;
        Q_GL_CALL(GL::GenBuffers(1, &id));
        BindObject(id);
        Q_GL_CALL(GL::BufferData(GL::ARRAY_BUFFER, size, nullptr, GL::DYNAMIC_DRAW));
        return VertexBuffer { id, size };
    }

    void VertexBuffer::DestroyObject(GraphicsID id) {
        Q_GL_CALL(GL::DeleteBuffers(1, &id));
    }

    void VertexBuffer::BindObject(GraphicsID id) {
        Q_GL_CALL(GL::BindBuffer(GL::ARRAY_BUFFER, id));
    }

    void VertexBuffer::UnbindObject() {
        Q_GL_CALL(GL::BindBuffer(GL::ARRAY_BUFFER, 0));
    }

    void VertexBuffer::SetDataBytes(Span<const byte> data) {
        Bind();
        Q_GL_CALL(GL::BufferSubData(GL::ARRAY_BUFFER, 0, (int)data.size_bytes(), data.data()));
    }

    void VertexBuffer::ClearData() {
        dataOffset = 0;
    }

    void VertexBuffer::AddDataBytes(Span<const byte> data) {
        Bind();
        Q_GL_CALL(GL::BufferSubData(GL::ARRAY_BUFFER, (int)dataOffset, (int)data.size_bytes(), data.data()));
        dataOffset += data.size_bytes();
    }
}
