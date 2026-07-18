#pragma once
#include "Comparison.h"
#include "Type.h"
#include "Option.h"
#include "Tuple.h"

namespace Quasi {
    template <class T, class Super> struct IIterator;

    template <class C>             using   CollectionItem = typename C::Item;
    template <class I>             concept IteratorAny    = Extends<I, IIterator<CollectionItem<I>, I>>;
    template <class I, class Item> concept Iterator       = IteratorAny<I> && ConvTo<CollectionItem<I>, Item>;

    template <class T, usize N> struct Array;

    inline struct IteratorEndMarker {} IteratorEnd;

#pragma region Iter Declarations
    namespace Iter {
        template <class It>          struct EnumerateIter;
        template <class It, class F> struct MapIter;
        template <class Vw>          struct SplitIter;
    }
#pragma endregion

    template <class T, class Super>
    struct ICollection {
        using Item = T;
        friend Super;
    protected:
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }

        Iterator<const RemRef<T>&> auto IterImpl() const = delete;
        Iterator<T&> auto IterMutImpl() = delete;
    public:
        Iterator<const RemRef<T>&> auto Iter() const { return super().IterImpl(); }
        Iterator<T&> auto IterMut() { return super().IterMutImpl(); }

        // Legacy methods:
        // CIter<Super> begin();
        // IteratorEndMarker end();
        Iterator<const RemRef<T>&> auto begin()  const { return Iter(); }
        Iterator<const RemRef<T>&> auto cbegin() const { return Iter(); }
        Iterator<T&> auto begin() { return IterMut(); }
        IteratorEndMarker end()  const { return IteratorEnd; }
        IteratorEndMarker cend() const { return IteratorEnd; }
    };

    template <class C>             concept CollectionAny = Implements<C, ICollection, CollectionItem<C>>;
    template <class C, class Item> concept Collection    = CollectionAny<C> && ConvTo<CollectionItem<C>, Item>;

    template <class T, class Super>
    struct IIterator : ICollection<T, Super> {
        using Item = T;
        friend Super;
        friend ICollection<T, Super>;
    protected:
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }

        Super IterImpl() const { return super(); }
        Super IterMutImpl() { return super(); }

        T CurrentImpl() const = delete;
        void AdvanceImpl() = delete;
        bool CanNextImpl() const = delete;
    public:
        Super IterMut() requires IsMutRef<T> { return super(); }

        T Current() const { return super().CurrentImpl(); }
        void Advance() { return super().AdvanceImpl(); }
        bool CanNext() const { return super().CanNextImpl(); }

        T operator*() const { return Current(); }
        Super& operator++() { Advance(); return super(); }
        friend bool operator==(const Super& it, const IteratorEndMarker&) { return !it.CanNext(); }
        friend bool operator!=(const Super& it, const IteratorEndMarker&) { return  it.CanNext(); }

        usize Count() const {
            usize count = 0;
            for (; CanNext(); Advance()) ++count;
            return count;
        }
        usize CountOccurs(Predicate<Item> auto&& pred) {
            usize count = 0;
            for (; CanNext(); Advance()) {
                if (pred(Current())) ++count;
            }
            return count;
        }
        usize CountNumberOf(const Item& i) { return CountOccurs(Cmp::Equals { i }); }

        usize AdvanceBy(usize n) {
            for (usize i = 0; i < n; ++i) {
                if (!CanNext()) return i;
                Advance();
            }
            return n;
        }

        OptStrong<T> Last() {
            OptStrong<T> last = nullptr;
            while (CanNext()) {
                last = Current();
                Advance();
            }
            return last;
        }

        OptStrong<T> Nth(usize n) {
            if (AdvanceBy(n) < n || !CanNext()) return nullptr;
            return Current();
        }

        void ForEach(Fn<void, const Item&> auto&& fn) {
            for (Item i : super())
                fn(i);
        }

        template <class R> Option<R> Reduce(Fn<R, R, const Item&> auto&& reducer) {
            if (!CanNext()) return nullptr;
            R starting = R(Current()); Advance();
            return Reduce((decltype(reducer))reducer, (R&&)starting);
        }
        template <class R> R Reduce(Fn<R, R, const Item&> auto&& reducer, R&& starting) {
            for (; CanNext(); Advance()) {
                starting = reducer((R&&)starting, Current());
            }
            return starting;
        }

        OptStrong<T> Min(const Fn<bool, const T&, const T&> auto& cmp = Cmp::LessThan {}, OptStrong<T> min = nullptr) {
            for (; CanNext(); Advance()) {
                Item curr = Current();
                if (!min || cmp(curr, *min)) min = (Item&&)curr;
            }
            return min;
        }
        OptStrong<T> Max() { return Min(Cmp::GreaterThan {}); }
        Option<Tuple<Strong<T>, Strong<T>>> MinMax() {
            if (!CanNext()) return nullptr;
            Strong<T> min = Current(), max = min;
            for (; CanNext(); Advance()) {
                Item curr = Current();
                if (curr < min) min = (Item&&)curr;
                if (curr > max) max = (Item&&)curr;
            }
            return Tuple<Strong<T>, Strong<T>> { min, max };
        }

        bool All(Predicate<Item> auto&& pred = Combinate::Identity {}) {
            for (; CanNext(); Advance())
                if (!pred(Current())) return false;
            return true;
        }
        bool Any(Predicate<Item> auto&& pred = Combinate::Identity {}) {
            for (; CanNext(); Advance())
                if (pred(Current())) return true;
            return false;
        }

        Option<Item> Find(Predicate<Item> auto&& pred) {
            for (; CanNext(); Advance()) {
                const Item i = Current();
                if (pred(i)) return i;
            }
            return nullptr;
        }

        template <Collection<RemQual<T>> C> C Collect() {
            C collection;
            for (; CanNext(); Advance()) {
                collection.Push(Current());
            }
            return collection;
        }

        template <usize N> Array<RemQual<T>, N> CollectFirstN() {
            Array<RemQual<T>, N> firstN;
            for (usize i = 0; i < N && CanNext(); ++i, Advance()) {
                firstN[i] = Current();
            }
            return firstN;
        }

        Iter::EnumerateIter<Super> Enumerate() const&;
        Iter::EnumerateIter<Super> Enumerate() &&;
        template <FnArgs<T> F> Iter::MapIter<Super, F> Map(F&& fn) const&;
        template <FnArgs<T> F> Iter::MapIter<Super, F> Map(F&& fn) &&;
        // TODO:
        // StepBy
        // Chain
        // ForEach
        // Filter
        // FilterMap
        // SkipWhile
        // TakeWhile
        // MapWhile
        // Take
        // Scan
        // FlatMap
        // Flatten
        // MapWindows
        // CollectPartitioned
        // Acc
    };


    inline struct WrapMarker {} Wrap;
    struct WrappingIndex { isize index; usize operator()(usize len) const { return (index % (isize)len + (isize)len) % (isize)len; } };

    WrappingIndex operator%(Integer auto i, WrapMarker) { return { (isize)i }; }
}
