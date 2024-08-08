#include "IndexBuffer.h"

#include <algorithm>
#include <vector>
#include <glp.h>

#include "GLDebug.h"

namespace Quasi::Graphics {
    IndexBuffer::IndexBuffer(GraphicsID id, u32 size) : GLObject(id), bufferSize(size) {}

    IndexBuffer IndexBuffer::New(u32 size) {
        GraphicsID id;
        Q_GL_CALL(GL::GenBuffers(1, &id));
        BindObject(id);
        Q_GL_CALL(GL::BufferData(GL::ELEMENT_ARRAY_BUFFER, sizeof(u32) * size, nullptr, GL::DYNAMIC_DRAW));
        return IndexBuffer { id, size };
    }

    void IndexBuffer::DestroyObject(GraphicsID id) {
        Q_GL_CALL(GL::DeleteBuffers(1, &id));
    }

    void IndexBuffer::BindObject(GraphicsID id) {
        Q_GL_CALL(GL::BindBuffer(GL::ELEMENT_ARRAY_BUFFER, id));
    }

    void IndexBuffer::UnbindObject() {
        Q_GL_CALL(GL::BindBuffer(GL::ELEMENT_ARRAY_BUFFER, 0));
    }

    void IndexBuffer::SetData(Span<const u32> data, u32 dOffset) {
        Bind();
        Q_GL_CALL(GL::BufferSubData(GL::ELEMENT_ARRAY_BUFFER, dOffset, data.size_bytes(), data.data()));
    }

    void IndexBuffer::ClearData() {
        dataOffset = 0;
    }

    void IndexBuffer::AddData(Span<const u32> data) {
        Bind();
        Q_GL_CALL(GL::BufferSubData(GL::ELEMENT_ARRAY_BUFFER, dataOffset * sizeof(uint), data.size() * sizeof(uint), data.data()));
        dataOffset += (u32)data.size();
    }
}
