#pragma once
#include "Utils/Span.h"

namespace Quasi::Iter {
    template <class T>
    struct Chunks : IIterator<const Span<T>, Chunks<T>> {
        using Item = const Span<T>;
        friend IIterator<Item, Chunks>;
    private:
        T* iter, *endIter;
        usize chunkSize = 1;
    public:
        explicit Chunks(T* iter, T* endIter, usize chunkSize) : iter(iter), endIter(endIter), chunkSize(chunkSize) {}
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
    Iter::Chunks<T> Span<T>::Chunks(usize chunkSize) {
        return Iter::Chunks<T> { data, data + size, chunkSize };
    }
    template <class T>
    Iter::Chunks<const T> Span<T>::Chunks(usize chunkSize) const {
        return Iter::Chunks<const T> { data, data + size, chunkSize };
    }
}