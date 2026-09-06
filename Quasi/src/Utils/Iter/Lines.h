#pragma once
#include "Utils/Str.h"

namespace Quasi::Iter {
    struct Lines : IIterator<const Str, Lines> {
        using Item = const Str;
        friend IIterator;
    private:
        Str source;
        usize i = -1;
        Lines(Str src) : source(src) { AdvanceImpl(); }
    protected:
        Str CurrentImpl() const;
        void AdvanceImpl();
        bool CanNextImpl() const;
    public:
        static Lines New(Str s) { return { s }; }
    };
}
