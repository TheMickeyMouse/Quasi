#pragma once
#include "Utils/Iterator.h"

namespace Quasi::Iter {
    template <class... It>
    struct Zip : IIterator<const Tuple<CollectionItem<It>...>, Zip<It...>> {
        using Item = const Tuple<CollectionItem<It>...>;
        friend IIterator<Item, Zip>;
    private:
        Tuple<It...> iters;
    public:
        Zip(It&&... iters) : iters(std::forward<It>(iters)...) {}
    protected:
        Item CurrentImpl() const {
            return iters.Apply([] (const It& ...xs) {
                return Item { (CollectionItem<It>&&)xs.Current()... };
            });
        }
        void AdvanceImpl() {
            iters.ApplyMut([] (It& ...xs) {
                (xs.Advance(), ...);
            });
        }
        bool CanNextImpl() const {
            return iters.Apply([] (const auto& ...xs) {
                return (... && xs.CanNext());
            });
        }
    };
}
