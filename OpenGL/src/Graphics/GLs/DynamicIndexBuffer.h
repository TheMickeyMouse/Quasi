#pragma once
#include "GLObject.h"


#include "NumTypes.h"
#include "TriIndices.h"

namespace Graphics {
    struct TriIndices;

    class DynamicIndexBuffer : public GLObject<BufferHandler<BufferType::INDEX>> {
    private:
        uint bufferSize = 0;
        uint dataOffset = 0;
        uint indexOffset = 0;
    public:
        DynamicIndexBuffer() = default;
        explicit DynamicIndexBuffer(uint size);

        void SetData(std::span<const uint> data);
        void SetData(std::span<const TriIndices> data) { SetData(stdu::span_cast<const uint>(data)); }
        template <class T> void SetData(const T& arr) { SetData(arr.data(), arr.size()); }

        void SetDataWhole(const uint* data);
        void SetDataWhole(const TriIndices* data) { SetDataWhole((const uint*)data); }
        template <class T> void SetDataWhole(const T& arr) { if (arr.size() >= bufferSize) SetDataWhole(arr.data()); }

        void ClearData(bool shallowClear = true);

        void AddData(std::span<const uint> data, int maxIndex = INT_MIN);
        void AddData(std::span<const TriIndices> data, int maxIndex = INT_MIN) { AddData(stdu::span_cast<const uint>(data), maxIndex); }
        template <stdu::array_like T> void AddData(const T& arr, int maxIndex = -1) { AddData(stdu::to_cspan(arr), maxIndex); }

        [[nodiscard]] uint GetLength() const { return bufferSize; }
        [[nodiscard]] uint GetUsedLength() const { return dataOffset; }

        friend class GraphicsDevice;
    };
}
