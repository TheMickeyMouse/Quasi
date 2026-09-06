#pragma once
#include "Utils/Iterator.h"

namespace Quasi::Iter {
    template <class View>
    struct Split : IIterator<const View, Split<View>> {
        friend IIterator<const View, Split>;
        using Item = const View;
    private:
        View source, separator;
        usize i = -1;
        Split(View src, View sep) : source(src), separator(sep) { AdvanceImpl(); }
    protected:
        View CurrentImpl() const { return source.First(i); }
        void AdvanceImpl() {
            if (i == source.Length()) {
                source.Advance(i);
                return;
            }
            source.Advance(i + separator.Length());
            for (i = 0; i < source.Length(); ++i) {
                if (source.Skip(i).StartsWith(separator)) return;
            }
        }
        bool CanNextImpl() const { return !source.IsEmpty(); }
    public:
        static Split New(View src, View sep) { return { src, sep }; }
    };
}
