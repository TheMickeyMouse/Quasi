#pragma once
#include "Utils/Iterator.h"

namespace Quasi::Iter {
    template <class T>
    struct EnumeratePair {
        usize index;
        T value;
    };

    template <class It>
    struct Enumerate : IIterator<const EnumeratePair<CollectionItem<It>>, Enumerate<It>> {
        using OriginalItem = CollectionItem<It>;
        using Item = const EnumeratePair<OriginalItem>;
        friend IIterator<Item, Enumerate>;
    private:
        usize i = 0;
        It iter;
    protected:
        Item CurrentImpl() const { return { i, iter.Current() }; }
        void AdvanceImpl() { iter.Advance(); ++i; }
        bool CanNextImpl() const { return iter.CanNext(); }
    public:
        explicit Enumerate(It it) : iter(std::move(it)) {}
    };
}