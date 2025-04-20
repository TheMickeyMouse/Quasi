#pragma once
#include "Iterator.h"
#include "Type.h"

namespace Quasi {
    template <class T>
    struct ZeroArray {
        operator T*() const { return nullptr; }
    };

    template <class T, usize N>
    struct Array : IContinuousCollection<T, Array<T, N>> {
        friend IContinuousCollection<T, Array>;
    private:
        using InnerRawArray = IfElse<N == 0, ZeroArray<T>, T[N]>;
        InnerRawArray array {};
    protected:
        T* DataImpl() { return array; }
        const T* DataImpl() const { return array; }
        usize LengthImpl() const { return N; }
    public:
        constexpr Array(const InnerRawArray& r) { Memory::RangeCopyNoOverlap(array, r, N); }
        constexpr Array(InnerRawArray&& r)      { Memory::RangeMoveNoOverlap(array, r, N); }
        constexpr Array(SimilarTo<T> auto&&... args) : array { args... } {}
    };

    template <class T, SimilarTo<T>... U>
    Array(T&&, U&&...) -> Array<RemQual<T>, 1 + sizeof...(U)>;
    template <class T, usize N>
    Array(T(&&)[N]) -> Array<RemQual<T>, N>;

    namespace Arrays {
        template <class A>
        Array<ArrayElement<A>, ArrayLength<A>> FromCArray(A&& array) {
            return { array };
        }
    }
}
