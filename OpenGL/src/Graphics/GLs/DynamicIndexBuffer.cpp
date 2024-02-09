#include "DynamicIndexBuffer.h"

#include <algorithm>
#include <vector>
#include <GL/glew.h>

namespace Graphics {
    DynamicIndexBuffer::DynamicIndexBuffer(uint size) : GLObject({}), bufferSize(size) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * size, nullptr, GL_DYNAMIC_DRAW);  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    }

    void DynamicIndexBuffer::SetData(const uint* data, uint size) {
        Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(uint), data);  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    }

    void DynamicIndexBuffer::SetDataWhole(const uint* data) {
        Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize * sizeof(uint), data);  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    }

    void DynamicIndexBuffer::ClearData(bool shallowClear) {
        Bind();
        dataOffset = 0;
        indexOffset = 0;
        if (shallowClear) return;
        const std::vector<uint> clear { bufferSize, 0 };
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize * sizeof(uint), clear.data());  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    }

    void DynamicIndexBuffer::AddData(const uint* data, uint size, int maxIndex) {
        std::vector<uint> dataOff(size);
        const uint indexOff = indexOffset;
        std::transform(data, data + size, dataOff.begin(), [=](uint i){ return i + indexOff; } );
        maxIndex = maxIndex == INT_MIN ? *std::ranges::max_element(dataOff) : (maxIndex + indexOffset);  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, dataOffset * sizeof(uint), size * sizeof(uint), dataOff.data());  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
        dataOffset += size;
        indexOffset = maxIndex + 1;
    }
}
