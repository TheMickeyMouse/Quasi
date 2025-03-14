#pragma once
#include "Utils/Iterator.h"

namespace Quasi::Iter {
    template <class View>
    struct SplitIter : IIterator<View, SplitIter<View>> {
        using Item = View;
        View source, separator;
        usize i = 0;
    private:
        SplitIter(View src, View sep) : source(src), separator(sep) {}
    protected:
        View CurrentImpl() const { return source.First(i); }
        void AdvanceImpl() {
            source.Advance(i + separator.Length());
            for (; i < source.Length(); ++i) {
                if (source.StartsWith(separator)) return;
            }
        }
        bool CanNextImpl() const { return i < source.Length(); }
    public:
        static SplitIter New(View src, View sep) { return { src, sep }; }
    };
}
