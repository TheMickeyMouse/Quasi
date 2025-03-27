#pragma once
#include "Numeric.h"

namespace Quasi::Bitwise {
    bool ContainsNullByte(u64 x);
    u32 PlaceOfNullByte(u64 x); // this uses big endian: 01234567 where 0 is the highest position, && assumes there is a null byte

    u32 BitCompress32(u32 origin, u32 mask);
    u32 BitExpand32  (u32 origin, u32 mask);
    u64 BitCompress64(u64 origin, u64 mask);
    u64 BitExpand64  (u64 origin, u64 mask);
}
