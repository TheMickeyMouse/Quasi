#pragma once

#include "GLObject.h"

namespace Quasi::Graphics {
    struct VertexDebugTypeInfo;
    class VertexBuffer : public GLObject<BufferHandler<BufferType::VERTEX>> {
    private:
        u32 dataOffset = 0;
        u32 bufferSize = 0;

        u32 vertSize = 1;
    public:
        VertexBuffer() = default;
        explicit VertexBuffer(u32 size, u32 typeSize);

        template <class T> static VertexBuffer of(u32 size) {
            return VertexBuffer(size, sizeof(T));
        }

        [[nodiscard]] u32 GetLength() const { return bufferSize; }

        void SetDataBytes(Span<const byte> data);
        template <class T> void SetData(Span<const T> data) { SetDataBytes(BytesOf(data)); }
        template <ArrayLike T> void SetData(const T& data) { SetData(TakeSpan(data)); }

        void ClearData();

        void AddDataBytes(Span<const byte> data);
        template <class T> void AddData(Span<const T> data) { AddDataBytes(BytesOf(data)); }
        template <ArrayLike T> void AddData(const T& data) { AddData(TakeSpan(data)); }

        friend class GraphicsDevice;
    };
}
