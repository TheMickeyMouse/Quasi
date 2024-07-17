#pragma once
#include "GLObject.h"

#include "TriIndices.h"

namespace Quasi::Graphics {
    struct TriIndices;

    class IndexBuffer : public GLObject<IndexBuffer> {
    private:
        u32 bufferSize = 0;
        u32 dataOffset = 0;

        explicit IndexBuffer(GraphicsID id, u32 size);
    public:
        IndexBuffer() = default;
        static IndexBuffer New(u32 size);
        static void DestroyObject(GraphicsID id);
        static void BindObject(GraphicsID id);
        static void UnbindObject();

        void SetData(Span<const u32> data, u32 dOffset = 0);
        void SetData(Span<const TriIndices> data, u32 dOffset = 0) { SetData(CastSpan<const u32>(data), dOffset); }

        void ClearData(); // this doesnt actually clear the dataz, just makes it not

        void AddData(Span<const u32> data);
        void AddData(Span<const TriIndices> data) { AddData(CastSpan<const u32>(data)); }

        [[nodiscard]] u32 GetLength() const { return bufferSize; }
        [[nodiscard]] u32 GetUsedLength() const { return dataOffset; }

        friend class GraphicsDevice;
    };
}
