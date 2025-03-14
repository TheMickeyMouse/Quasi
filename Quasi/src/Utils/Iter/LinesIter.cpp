#include "LinesIter.h"

#include "Utils/Bitwise.h"

namespace Quasi::Iter {
    LinesIter::Item LinesIter::CurrentImpl() const {
        return source.First(i);
    }

    void LinesIter::AdvanceImpl() {
        source.Advance(i + 1);
        for (; i < source.Length() - 8; i += 8) {
            u64 bytes = Memory::ReadU64(source.Data() + i);
            bytes -= '\n' * 0x0101010101010101;
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

    bool LinesIter::CanNextImpl() const {
        return i < source.Length();
    }
}
