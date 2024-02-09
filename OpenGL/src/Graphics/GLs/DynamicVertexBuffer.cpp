#include "DynamicVertexBuffer.h"

namespace Graphics {
    DynamicVertexBuffer::DynamicVertexBuffer(uint size, uint typeSize, std::type_index type)
        : GLObject({}), bufferSize(size), vertSize(typeSize), vertType(type) {
        GLCALL(glBufferData(GL_ARRAY_BUFFER, typeSize * size, nullptr, GL_DYNAMIC_DRAW));
    }
    
    void DynamicVertexBuffer::SetDataUnchecked(const void* data, uint vertexSize, uint count) {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, (int)(count * vertexSize), data);
    }

    void DynamicVertexBuffer::ClearData(bool shallowClear) {
        Bind();
        dataOffset = 0;
        if (shallowClear) return;
        const std::vector<uchar> clear(bufferSize * vertSize, 0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (int)(bufferSize * vertSize), clear.data());
    }

    void DynamicVertexBuffer::AddDataUnchecked(const void* data, uint vertexSize, uint count) {
        glBufferSubData(GL_ARRAY_BUFFER, (int)(dataOffset * vertexSize), (int)(count * vertexSize), data);
        dataOffset += count;
    }
}