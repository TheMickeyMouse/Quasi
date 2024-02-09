#pragma once
#include <limits.h>
#include <utility>

#include "GLObject.h"
#include "opengl.h"

#include "NumTypes.h"

namespace Graphics {
    struct TriIndices;

    class DynamicIndexBuffer : public GLObject<BufferHandler<BufferType::INDEX>> {
    private:
        uint bufferSize = 0;
        uint dataOffset = 0;
        uint indexOffset = 0;
    public:
        DynamicIndexBuffer() = default;
        OPENGL_API explicit DynamicIndexBuffer(uint size);

        OPENGL_API void SetData(const uint* data, uint size);
        OPENGL_API void SetData(const TriIndices* data, uint size) { SetData((const uint*)data, size * 3); }
        template <class T> void SetData(const T& arr) { SetData(arr.data(), arr.size()); }

        OPENGL_API void SetDataWhole(const uint* data);
        OPENGL_API void SetDataWhole(const TriIndices* data) { SetDataWhole((const uint*)data); }
        template <class T> void SetDataWhole(const T& arr) { if (arr.size() >= bufferSize) SetDataWhole(arr.data()); }

        OPENGL_API void ClearData(bool shallowClear = true);

        OPENGL_API void AddData(const uint* data, uint size, int maxIndex = INT_MIN);
        OPENGL_API void AddData(const TriIndices* data, uint size, int maxIndex = INT_MIN) { AddData((const uint*)data, size * 3, maxIndex); }
        template <class T> void AddData(const T& arr, int maxIndex = -1) { AddData(arr.data(), arr.size(), maxIndex); }

        [[nodiscard]] uint GetLength() const { return bufferSize; }
        [[nodiscard]] uint GetUsedLength() const { return dataOffset; }
    };
}
