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
        explicit Enumerate(It it) : iter(std::move(it)) {}
    protected:
        Item CurrentImpl() const { return { i, iter.Current() }; }
        void AdvanceImpl() { iter.Advance(); ++i; }
        bool CanNextImpl() const { return iter.CanNext(); }
    public:
        static Enumerate New(It it) { return Enumerate { it }; }
    };

    namespace Enumerators {
        template <IteratorAny It>
        static Enumerate<It> Iter(It it) { return Enumerate<It>::New(std::move(it)); }
    }
}

namespace Quasi {
    template <class T, class Super>
    Iter::Enumerate<Super> IIterator<T, Super>::Enumerate() const& {
        return Iter::Enumerators::Iter(super());
    }

    template <class T, class Super>
    Iter::Enumerate<Super> IIterator<T, Super>::Enumerate() && {
        return Iter::Enumerators::Iter(std::move(super()));
    }
}