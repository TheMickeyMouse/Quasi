#pragma once

#include "GLObject.h"
#include "Utils/Span.h"

namespace Quasi::Graphics {
    class VertexBuffer : public GLObject<VertexBuffer> {
        u32 dataOffset = 0;
        u32 bufferSize = 0;

        explicit VertexBuffer(GraphicsID id, u32 size);
    public:
        VertexBuffer() = default;
        static VertexBuffer New(u32 size);
        static void DestroyObject(GraphicsID id);
        static void BindObject(GraphicsID id);
        static void UnbindObject();

        u32 GetLength() const { return bufferSize; }

        void SetDataBytes(Span<const byte> data);
        template <class T> void SetData(Span<const T> data) { SetDataBytes(data.AsBytes()); }
        template <ContinuousCollectionAny T> void SetData(const T& data) { SetData(data.AsSpan()); }

        void ClearData();

        void AddDataBytes(Span<const byte> data);
        template <class T> void AddData(Span<const T> data) { AddDataBytes(data.AsBytes()); }
        template <ContinuousCollectionAny T> void AddData(const T& data) { AddData(data.AsSpan()); }

        friend class GraphicsDevice;
    };
}
