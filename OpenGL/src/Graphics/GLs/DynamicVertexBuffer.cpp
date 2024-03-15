#include "DynamicVertexBuffer.h"

#include <vector>
#include <GL/glew.h>

#include "GLDebug.h"

namespace Graphics {
    DynamicVertexBuffer::DynamicVertexBuffer(uint size, uint typeSize, std::type_index type)
        : GLObject({}), bufferSize(size), vertSize(typeSize), vertType(type) {
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, typeSize * size, nullptr, GL_DYNAMIC_DRAW));
    }
    
    void DynamicVertexBuffer::SetDataUnchecked(stdu::cbyte_span data) {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, (int)data.size_bytes(), data.data());
    }

    void DynamicVertexBuffer::ClearData(bool shallowClear) {
        Bind();
        dataOffset = 0;
        if (shallowClear) return;
        const std::vector<uchar> clear(bufferSize * vertSize, 0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (int)(bufferSize * vertSize), clear.data());
    }

    void DynamicVertexBuffer::AddDataUnchecked(stdu::cbyte_span data) {
        glBufferSubData(GL_ARRAY_BUFFER, (int)(dataOffset * vertSize), (int)data.size_bytes(), data.data());
        dataOffset += (uint)(data.size_bytes() / vertSize);
    }
}
