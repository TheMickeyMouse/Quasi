#pragma once
#include "Type.h"

namespace Quasi {
    struct ArenaAllocator {
        usize sizeBytes, dataOffset;
        byte* data; // void yields warning

        ArenaAllocator(usize bytes);
        ~ArenaAllocator();

        ArenaAllocator(const ArenaAllocator&) = delete;
        ArenaAllocator& operator=(const ArenaAllocator&) = delete;
        ArenaAllocator(ArenaAllocator&&) noexcept;
        ArenaAllocator& operator=(ArenaAllocator&&) noexcept;

        void Clear();

        void* AllocBytes(usize bytes);
        template <class T> T* Alloc() {
            // Debug("allocating {} ({} bytes)", Text::TypeName<T>(), sizeof(T));
            return (T*)AllocBytes(sizeof(T));
        }

        template <class T, class... R> T* Create(R&&... args) {
            T* object = Alloc<T>();
            new (object) T(std::forward<R>(args)...);
            return object;
        }
    };
} // Q
