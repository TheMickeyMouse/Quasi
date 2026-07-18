#pragma once
#include "Utils/Span.h"

namespace Quasi::Iter {
    template <class T>
    struct ChunksIter : IIterator<const Span<T>, ChunksIter<T>> {
        using Item = const Span<T>;
        friend IIterator<Item, ChunksIter>;
    private:
        T* iter, *endIter;
        usize chunkSize = 1;
    public:
        explicit ChunksIter(T* iter, T* endIter, usize chunkSize) : iter(iter), endIter(endIter), chunkSize(chunkSize) {}
    protected:
        Item CurrentImpl() const {
            return Spans::Slice(iter, std::min((usize)(endIter - iter), chunkSize));
        }
        void AdvanceImpl() {
            iter += chunkSize;
        }
        bool CanNextImpl() const { return iter < endIter; }
    };
}

namespace Quasi {
    template <class T>
    Iter::ChunksIter<T> Span<T>::Chunks(usize chunkSize) {
        return Iter::ChunksIter<T> { data, data + size, chunkSize };
    }
    template <class T>
    Iter::ChunksIter<const T> Span<T>::Chunks(usize chunkSize) const {
        return Iter::ChunksIter<const T> { data, data + size, chunkSize };
    }
}