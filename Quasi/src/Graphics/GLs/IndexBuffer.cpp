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
        indexOffset = 0;
    }

    void IndexBuffer::AddData(Span<const u32> data, u32 maxIndex) {
        Bind();
        Vec<u32> dataOff(data.size());
        std::ranges::transform(data, dataOff.begin(), [iOff = indexOffset](u32 i){ return i + iOff; } );
        maxIndex = maxIndex == ~0 ? *std::ranges::max_element(dataOff) : (maxIndex + indexOffset);
        Q_GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dataOffset * sizeof(uint), data.size() * sizeof(uint), dataOff.data()));
        dataOffset += (u32)data.size();
        indexOffset = maxIndex + 1;
    }
}
