#include "ArenaAllocator.h"

#include "Debug/Logger.h"

namespace Quasi {
    ArenaAllocator::ArenaAllocator(usize bytes)
            : sizeBytes(bytes), dataOffset(0), data((byte*)malloc(bytes)) {}

    ArenaAllocator::~ArenaAllocator() {
        free(data);
    }

    ArenaAllocator::ArenaAllocator(ArenaAllocator&& alloc) noexcept {
        data = alloc.data;
        alloc.data = nullptr;
        sizeBytes = alloc.sizeBytes;
        dataOffset = alloc.dataOffset;
    }

    ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& alloc) noexcept {
        free(data);
        data = alloc.data;
        alloc.data = nullptr;
        sizeBytes = alloc.sizeBytes;
        dataOffset = alloc.dataOffset;
        return *this;
    }

    void ArenaAllocator::Clear() {
        dataOffset = 0;
    }

    void* ArenaAllocator::AllocBytes(usize bytes) {
        Debug::AssertFmt(dataOffset + bytes < sizeBytes, "allocation too much memory: {} + {} > {} (bytes)", dataOffset, bytes, sizeBytes);
        void* allocated = data + dataOffset;
        dataOffset += bytes;
        return allocated;
    }
} // Q