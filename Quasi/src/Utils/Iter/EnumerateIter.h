#pragma once
#include "Utils/Iterator.h"

namespace Quasi::Iter {
    template <class It>
    struct EnumerateIter : IIterator<Tuple<usize, CollectionItem<It>>, EnumerateIter<It>> {
        using OriginalItem = CollectionItem<It>;
        using Item = Tuple<usize, OriginalItem>;
        friend IIterator<Item, EnumerateIter>;
    private:
        usize i = 0;
        It iter;
        explicit EnumerateIter(It it) : iter(std::move(it)) {}
    protected:
        Item CurrentImpl() const { return { i, iter.Current() }; }
        void AdvanceImpl() { iter.Advance(); }
        bool CanNextImpl() const { return iter.CanNext(); }
    public:
        static EnumerateIter New(It it) { return EnumerateIter { it }; }
    };

    namespace Enumerators {
        template <IteratorAny It>
        static EnumerateIter<It> Iter(It it) { return EnumerateIter<It>::New(std::move(it)); }
    }
}

namespace Quasi {
    template <class T, class Super>
    Iter::EnumerateIter<Super> IIterator<T, Super>::Enumerate() const& {
        return Iter::Enumerators::Iter(super());
    }

    template <class T, class Super>
    Iter::EnumerateIter<Super> IIterator<T, Super>::Enumerate() && {
        return Iter::Enumerators::Iter(std::move(super()));
    }
}