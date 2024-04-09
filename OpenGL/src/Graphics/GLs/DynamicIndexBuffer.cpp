#include "DynamicIndexBuffer.h"

#include <algorithm>
#include <vector>
#include <GL/glew.h>

#include "GLDebug.h"

namespace Graphics {
    DynamicIndexBuffer::DynamicIndexBuffer(uint size) : GLObject({}), bufferSize(size) {
        GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * size, nullptr, GL_DYNAMIC_DRAW));
    }

    void DynamicIndexBuffer::SetData(std::span<const uint> data) {
        Bind();
        GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, data.size_bytes(), data.data()));
    }

    void DynamicIndexBuffer::SetDataWhole(const uint* data) {
        Bind();
        GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize * sizeof(uint), data));
    }

    void DynamicIndexBuffer::ClearData(bool shallowClear) {
        Bind();
        dataOffset = 0;
        indexOffset = 0;
        if (shallowClear) return;
        const std::vector<uint> clear { bufferSize, 0 };
        GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize * sizeof(uint), clear.data()));
    }

    void DynamicIndexBuffer::AddData(std::span<const uint> data, int maxIndex) {
        Bind();
        std::vector<uint> dataOff(data.size());
        const uint indexOff = indexOffset;
        std::ranges::transform(data, dataOff.begin(), [=](uint i){ return i + indexOff; } );
        maxIndex = maxIndex == INT_MIN ? *std::ranges::max_element(dataOff) : (maxIndex + indexOffset);
        GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dataOffset * sizeof(uint), data.size() * sizeof(uint), dataOff.data()));
        dataOffset += (uint)data.size();
        indexOffset = maxIndex + 1;
    }
}
