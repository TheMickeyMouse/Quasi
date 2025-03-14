#pragma once
#include "Utils/Str.h"

namespace Quasi::Iter {
    struct LinesIter : IIterator<Str, LinesIter> {
        using Item = Str;
        Str source;
        usize i = 0;
    private:
        LinesIter(Str src) : source(src) {}
    protected:
        Item CurrentImpl() const;
        void AdvanceImpl();
        bool CanNextImpl() const;
    public:
        static LinesIter New(Str s) { return { s }; }
    };
}
