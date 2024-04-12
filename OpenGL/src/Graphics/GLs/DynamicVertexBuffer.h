#pragma once
#include "NumTypes.h"
#include <core.h>

#include <typeindex>
#include <span>

#include "GLDebug.h"
#include "GLObject.h"

namespace Graphics {
    struct VertexDebugTypeInfo;
    class DynamicVertexBuffer : public GLObject<BufferHandler<BufferType::VERTEX>> {
    private:
        uint dataOffset = 0;
        uint bufferSize = 0;

        uint vertSize = 1;
        VertexDebugTypeIndex vertType;

        template <class T> bool IsValidVert() {
            if constexpr (Debug::Logger::DEBUG) {
                if (VertexDebugTypeInfo::of<T>() == vertType) return true;
                GLLogger().Error({"Vertex type {} doesn't fit type {}"}, stdu::nameof<T>(), vertType->name);
                return false;
            } else return true;
        }
    public:
        DynamicVertexBuffer() = default;
        OPENGL_API explicit DynamicVertexBuffer(uint size, uint typeSize, VertexDebugTypeIndex type);

        template <class T> static DynamicVertexBuffer of(uint size) {
            return DynamicVertexBuffer(size, sizeof(T), typeid(T));
        }

        [[nodiscard]] uint GetLength() const { return bufferSize; }
        [[nodiscard]] VertexDebugTypeIndex GetType() const { return vertType; }

        OPENGL_API void SetDataUnchecked(stdu::cbyte_span data);
        template <class T> void SetData(std::span<const T> data);
        template <stdu::array_like T> void SetData(const T& arr) { SetData(stdu::to_cspan(arr)); }

        OPENGL_API void ClearData(bool shallowClear = true);

        OPENGL_API void AddDataUnchecked(stdu::cbyte_span data);
        template <class T> void AddData(std::span<const T> data);
        template <stdu::array_like T> void AddData(const T& arr) { AddData(stdu::to_cspan(arr)); }

        friend class GraphicsDevice;
    };

    template <class T>
    void DynamicVertexBuffer::SetData(std::span<const T> data) {
        if (IsValidVert<T>()) SetDataUnchecked(stdu::span_cast<const uchar>(data));
    }

    template <class T>
    void DynamicVertexBuffer::AddData(std::span<const T> data) {
        if (IsValidVert<T>()) AddDataUnchecked(stdu::span_cast<const uchar>(data));
    }
}
