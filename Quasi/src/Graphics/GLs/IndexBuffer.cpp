#include "IndexBuffer.h"

#include <algorithm>
#include <vector>
#include <GL/glew.h>

#include "GLDebug.h"

namespace Quasi::Graphics {
    IndexBuffer::IndexBuffer(u32 size) : bufferSize(size) {
        Create();
        Q_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * size, nullptr, GL_DYNAMIC_DRAW));
    }

    void IndexBuffer::SetData(Span<const u32> data, u32 dOffset) {
        Bind();
        Q_GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dOffset, data.size_bytes(), data.data()));
    }

    void IndexBuffer::ClearData() {
        dataOffset = 0;
    }

    void IndexBuffer::AddData(Span<const u32> data) {
        Bind();
        Q_GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dataOffset * sizeof(uint), data.size() * sizeof(uint), data.data()));
        dataOffset += (u32)data.size();
    }
}
