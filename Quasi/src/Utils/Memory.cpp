#include "Memory.h"

namespace Quasi {
    void* Memory::AllocateRaw(usize size) {
        return ::operator new (size);
    }

    void Memory::FreeRaw(void* mem) {
        return ::operator delete (mem);
    }

    u16 Memory::ReadU16(const void* bytes) { auto b = (const byte*)bytes; return b[0] <<  8 | b[1]; }
    u32 Memory::ReadU32(const void* bytes) { auto b = (const byte*)bytes; return b[0] << 24 | b[1] << 16 | b[2] <<  8 | b[3]; }
    u64 Memory::ReadU64(const void* bytes) { auto b = (const byte*)bytes; return b[0] << 56 | b[1] << 48 | b[2] << 40 | b[3] << 32 |
                                                                                 b[4] << 24 | b[5] << 16 | b[6] <<  8 | b[7]; }
    i16 Memory::ReadI16(const void* bytes) { return (i16)ReadU16(bytes); }
    i32 Memory::ReadI32(const void* bytes) { return (i32)ReadU32(bytes); }
    i64 Memory::ReadI64(const void* bytes) { return (i64)ReadU64(bytes); }

    u16 Memory::ReadU16Little(const void* bytes) { auto b = (const byte*)bytes; return b[1] <<  8 | b[0]; }
    u32 Memory::ReadU32Little(const void* bytes) { auto b = (const byte*)bytes; return b[3] << 24 | b[2] << 16 | b[1] <<  8 | b[0]; }
    u64 Memory::ReadU64Little(const void* bytes) { auto b = (const byte*)bytes; return b[7] << 56 | b[6] << 48 | b[5] << 40 | b[4] << 32 |
                                                                                       b[3] << 24 | b[2] << 16 | b[1] <<  8 | b[0]; }
    i16 Memory::ReadI16Little(const void* bytes) { return (i16)ReadU16Little(bytes); }
    i32 Memory::ReadI32Little(const void* bytes) { return (i32)ReadU32Little(bytes); }
    i64 Memory::ReadI64Little(const void* bytes) { return (i64)ReadU64Little(bytes); }

    void Memory::MemCopy(void* out, const void* in, usize bytes) {
        for (usize i = 0; i < bytes; ++i)
            ((byte*)out)[i] = ((byte*)in)[i];
    }

    void Memory::MemCopyNoOverlap(void* out, const void* in, usize bytes) {
        memcpy(out, in, bytes);
    }

    void Memory::MemCopyRev(void* out, const void* in, usize bytes) {
        for (usize i = bytes; i --> 0; )
            ((byte*)out)[i] = ((byte*)in)[i];
    }
}
