#include <cstring>

#include "Memory.h"

namespace Quasi {
    void* Memory::AllocateRaw(usize size) {
        return ::operator new (size);
    }

    void Memory::FreeRaw(void* mem) {
        return ::operator delete (mem);
    }

    u16 Memory::ReadU16(const void* bytes) { const auto b = (const byte*)bytes; return b[0] <<  8 | b[1]; }
    u32 Memory::ReadU32(const void* bytes) { const auto b = (const byte*)bytes; return b[0] << 24 | b[1] << 16 | b[2] <<  8 | b[3]; }
    u64 Memory::ReadU64(const void* bytes) { const auto b = (const byte*)bytes; return b[0] << 56 | b[1] << 48 | b[2] << 40 | b[3] << 32 |
                                                                                       b[4] << 24 | b[5] << 16 | b[6] <<  8 | b[7]; }
    i16 Memory::ReadI16(const void* bytes) { return (i16)ReadU16(bytes); }
    i32 Memory::ReadI32(const void* bytes) { return (i32)ReadU32(bytes); }
    i64 Memory::ReadI64(const void* bytes) { return (i64)ReadU64(bytes); }

    u16 Memory::ReadU16Little(const void* bytes) { const auto b = (const byte*)bytes; return b[1] <<  8 | b[0]; }
    u32 Memory::ReadU32Little(const void* bytes) { const auto b = (const byte*)bytes; return b[3] << 24 | b[2] << 16 | b[1] <<  8 | b[0]; }
    u64 Memory::ReadU64Little(const void* bytes) { const auto b = (const byte*)bytes; return b[7] << 56 | b[6] << 48 | b[5] << 40 | b[4] << 32 |
                                                                                             b[3] << 24 | b[2] << 16 | b[1] <<  8 | b[0]; }
    i16 Memory::ReadI16Little(const void* bytes) { return (i16)ReadU16Little(bytes); }
    i32 Memory::ReadI32Little(const void* bytes) { return (i32)ReadU32Little(bytes); }
    i64 Memory::ReadI64Little(const void* bytes) { return (i64)ReadU64Little(bytes); }

    u16 Memory::ReadU16Native(const void* bytes) { u16 x; MemCopyNoOverlap(&x, bytes, sizeof(u16)); return x; }
    u32 Memory::ReadU32Native(const void* bytes) { u32 x; MemCopyNoOverlap(&x, bytes, sizeof(u32)); return x; }
    u64 Memory::ReadU64Native(const void* bytes) { u64 x; MemCopyNoOverlap(&x, bytes, sizeof(u64)); return x; }
    i16 Memory::ReadI16Native(const void* bytes) { i16 x; MemCopyNoOverlap(&x, bytes, sizeof(i16)); return x; }
    i32 Memory::ReadI32Native(const void* bytes) { i32 x; MemCopyNoOverlap(&x, bytes, sizeof(i32)); return x; }
    i64 Memory::ReadI64Native(const void* bytes) { i64 x; MemCopyNoOverlap(&x, bytes, sizeof(i64)); return x; }

    void Memory::WriteU16(u16 x, void* out) { const auto b = (byte*)out; b[0] = x >> 8;  b[1] = x; }
    void Memory::WriteU32(u32 x, void* out) { const auto b = (byte*)out; b[0] = x >> 24; b[1] = x >> 16; b[2] = x >> 8;  b[3] = x; }
    void Memory::WriteU64(u64 x, void* out) { const auto b = (byte*)out; b[0] = x >> 56; b[1] = x >> 48; b[2] = x >> 40; b[3] = x >> 32;
                                                                         b[4] = x >> 24; b[5] = x >> 16; b[6] = x >> 8;  b[7] = x; }

    void Memory::WriteI16(i16 x, void* out) { WriteU16((u16)x, out); }
    void Memory::WriteI32(i32 x, void* out) { WriteU32((u32)x, out); }
    void Memory::WriteI64(i64 x, void* out) { WriteU64((u64)x, out); }

    void Memory::WriteU16Little(u16 x, void* out) { const auto b = (byte*)out; b[1] = x >> 8;  b[0] = x; }
    void Memory::WriteU32Little(u32 x, void* out) { const auto b = (byte*)out; b[3] = x >> 24; b[2] = x >> 16; b[1] = x >> 8;  b[0] = x; }
    void Memory::WriteU64Little(u64 x, void* out) { const auto b = (byte*)out; b[7] = x >> 56; b[6] = x >> 48; b[5] = x >> 40; b[4] = x >> 32;
                                                                               b[3] = x >> 24; b[2] = x >> 16; b[1] = x >> 8;  b[0] = x; }

    void Memory::WriteI16Little(i16 x, void* out) { WriteU16Little((u16)x, out); }
    void Memory::WriteI32Little(i32 x, void* out) { WriteU32Little((u32)x, out); }
    void Memory::WriteI64Little(i64 x, void* out) { WriteU64Little((u64)x, out); }

    void Memory::WriteU16Native(u16 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(u16)); }
    void Memory::WriteU32Native(u32 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(u32)); }
    void Memory::WriteU64Native(u64 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(u64)); }
    void Memory::WriteI16Native(i16 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(i16)); }
    void Memory::WriteI32Native(i32 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(i32)); }
    void Memory::WriteI64Native(i64 x, void* out) { MemCopyNoOverlap(out, &x, sizeof(i64)); }

    u16 operator ""_u16(const char* s, usize n) {
        return n == 0 ? 0 :
               n == 1 ? s[0] : Memory::ReadU16(s);
    }

    u32 operator ""_u32(const char* s, usize n) {
        switch (n) {
            case 0: return 0;
            case 1: return s[0];
            case 2: return s[0] << 8  | s[1];
            case 3: return s[0] << 16 | s[1] << 8 | s[2];
            default: return Memory::ReadU32(s);
        }
    }

    u64 operator ""_u64(const char* s, usize n) {
        switch (n) {
            case 0: return 0;
            case 1: return s[0];
            case 2: return s[0] << 8  | s[1];
            case 3: return s[0] << 16 | s[1] << 8  | s[2];
            case 4: return s[0] << 24 | s[1] << 16 | s[2] << 8  | s[3];
            case 5: return s[0] << 32 | s[1] << 24 | s[2] << 16 | s[3] << 8  | s[4];
            case 6: return s[0] << 40 | s[1] << 32 | s[2] << 24 | s[3] << 16 | s[4] << 8  | s[5];
            case 7: return s[0] << 48 | s[1] << 40 | s[2] << 32 | s[3] << 24 | s[4] << 16 | s[5] << 8 | s[6];
            default: return Memory::ReadU64(s);
        }
    }

    u16 Memory::ByteSwap16(u16 x) { return (x << 8) | (x >> 8); }
    u32 Memory::ByteSwap32(u32 x) {
        x     = x << 16 | x >> 16;
        return (x & 0xFF00FF00) >> 8 | (x & 0x00FF00FF) << 8;
    }
    u64 Memory::ByteSwap64(u64 x) {
        x    =  x << 32 | x >> 32;
        x    = (x & 0xFFFF0000FFFF0000) >> 16 | (x & 0x0000FFFF0000FFFF) << 16;
        return (x & 0xFF00FF00FF00FF00) >> 8  | (x & 0x00FF00FF00FF00FF) << 8;
    }

    void Memory::MemCopy(void* out, const void* in, usize bytes) {
        for (usize i = 0; i < bytes; ++i)
            ((byte*)out)[i] = ((byte*)in)[i];
    }

    void Memory::MemCopyNoOverlap(void* out, const void* in, usize bytes) {
        std::memcpy(out, in, bytes);
    }

    void Memory::MemCopyRev(void* out, const void* in, usize bytes) {
        for (usize i = bytes; i --> 0; )
            ((byte*)out)[i] = ((byte*)in)[i];
    }
}
