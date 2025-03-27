#include "Bitwise.h"

#include <bmi2intrin.h>

namespace Quasi::Bitwise {
    bool ContainsNullByte(u64 x) {
        // theres an explanation here: https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
        static constexpr u64 EXCEPT_FIRST = 0x7F7F7F7F'7F7F7F7F;
        return ~(((x & EXCEPT_FIRST) + EXCEPT_FIRST) | x | EXCEPT_FIRST);
    }

    u32 PlaceOfNullByte(u64 x) {
        static constexpr u64 EXCEPT_FIRST = 0x7F7F7F7F'7F7F7F7F;
        const u64 q = ~(((x & EXCEPT_FIRST) + EXCEPT_FIRST) | x | EXCEPT_FIRST);
        return q ? u64s::CountLeftZeros(q) >> 3 : 8;
    }

    u32 BitCompress32(u32 origin, u32 mask) {
        return _pext_u32(origin, mask);
    }

    u32 BitExpand32(u32 origin, u32 mask) {
        return _pdep_u32(origin, mask);
    }

    u64 BitCompress64(u64 origin, u64 mask) {
        return _pext_u64(origin, mask);
    }

    u64 BitExpand64(u64 origin, u64 mask) {
        return _pdep_u64(origin, mask);
    }
}
