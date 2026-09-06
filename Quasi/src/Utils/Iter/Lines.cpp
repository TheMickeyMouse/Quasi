#include "Lines.h"

#include "Utils/Bitwise.h"

namespace Quasi::Iter {
    Str Lines::CurrentImpl() const {
        return source.First(i);
    }

    void Lines::AdvanceImpl() {
        if (i == source.Length()) {
            source.Advance(i);
            return;
        }
        source.Advance(i + 1);
        for (i = 0; i + 8 < source.Length(); i += 8) {
            u64 bytes = Memory::ReadU64(source.Data() + i);
            bytes ^= '\n' * 0x0101010101010101;
            if (const u32 position = Bitwise::PlaceOfNullByte(bytes); position != 8) {
                i += position;
                return;
            }
        }
        for (; i < source.Length(); ++i) {
            if (source[i] == '\n') {
                return;
            }
        }
    }

    bool Lines::CanNextImpl() const {
        return !source.IsEmpty();
    }
}
