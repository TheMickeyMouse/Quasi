#include "DynamicIndexBuffer.h"

#include <algorithm>
#include <vector>
#include <GL/glew.h>

namespace Graphics {
    DynamicIndexBuffer::DynamicIndexBuffer(uint size) : bufferSize(size) {
        glGenBuffers(1, &rendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * size, nullptr, GL_DYNAMIC_DRAW);  // NOLINT(bugprone-narrowing-conversions, cppcoreguidelines-narrowing-conversions)
    }

    DynamicIndexBuffer::~DynamicIndexBuffer() {
        glDeleteBuffers(1, &rendererID);
    }

    void DynamicIndexBuffer::Transfer(DynamicIndexBuffer& dest, DynamicIndexBuffer&& from) {
        dest.rendererID = from.rendererID;
        from.rendererID = GL_NULL;

        dest.bufferSize = from.bufferSize;
        dest.dataOffset = from.dataOffset;
        dest.indexOffset = from.indexOffset;
    }

    void DynamicIndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID);
    }

    void DynamicIndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
