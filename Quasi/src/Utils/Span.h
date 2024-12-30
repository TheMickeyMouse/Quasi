#pragma once
#include "ArrayBox.h"
#include "Memory.h"
#include "Iterator.h"
#include "Option.h"
#include "Ref.h"

namespace Quasi {
    namespace Algorithm {};
    template <class T> struct BufferIterator;
    template <class T, usize N> struct Array;

#define mut requires IsMut<T>
    template <class T>
    struct Span : IContinuousCollection<T, Span<T>> {
        friend ICollection<T, Span>;
        friend IContinuousCollection<T, Span>;
        template <class U> friend struct Span;
        using MutT = RemConst<T>;
    private:
        T* data;
        usize size;

        Span(T* dat, usize len) : data(dat), size(len) {}
    public:
        Span() : data(nullptr), size(0) {}
        template <usize N>
        Span(T (&arr)[N]) : data(arr), size(N) {}
        template <ContinuousCollection<RemConst<T>> Coll>
        Span(const Coll& collection) : data(collection.Data()), size(collection.Length()) {}
        template <ContinuousCollection<T> Coll>
        Span(Coll& collection) mut : data(collection.Data()), size(collection.Length()) {}

        template <usize N> static Span FromArray(T (&arr) [N]) { return { arr, N }; }
        static Span FromBuffer(T* dat, usize len) { return { dat, len }; }
        static Span Single(T& obj) { return { &obj, 1 }; }
        static Span Empty() { return {}; }

        template <IsMut U> requires SameAs<T, byte>
        static Span BytesOf(U& memRead)       { return { Memory::DowncastPtr(&memRead), sizeof(U) }; }
        template <class U> requires SameAs<T, const byte>
        static Span BytesOf(const U& memRead) { return { Memory::DowncastPtr(&memRead), sizeof(U) }; }

        Str              AsStr()   const requires SameAs<const T, const char> { return { data, size }; }
        Span<byte>       AsBytes() mut   { return { Memory::AsBytePtr(data), size * sizeof(T) }; }
        Span<const byte> AsBytes() const { return { Memory::AsBytePtr(data), size * sizeof(T) }; }
        Span<const T>    AsSpan()  const { return *this; }
        Span             AsSpan()  mut   { return *this; }
        // Range<T*> AsPtrRange()

    protected:
        usize LengthImpl() const { return size; }

        T* DataImpl() { return data; }
        const T* DataImpl() const { return data; }
    public:
        BufferIterator<T&>       IterMut() mut { return { data, data + size }; }
        BufferIterator<const T&> Iter()  const { return { data, data + size }; }
        BufferIterator<T>        IntoIter()    { return { data, data + size }; }

        usize ByteSize()   const { return size * sizeof(T); }
        bool IsEmpty()     const { return size == 0; }
        operator bool()    const { return !IsEmpty(); }

        MutT& FirstMut() mut   { return data[0]; }
        MutT& LastMut()  mut   { return data[size - 1]; }
        const T& First() const { return data[0]; }
        const T& Last()  const { return data[size - 1]; }

        Span<MutT> SubspanMut(usize start)              mut { return { data + start, size - start }; }
        Span<MutT> SubspanMut(usize start, usize count) mut { return { data + start, count }; }
        Span<MutT> FirstMut(usize num)                  mut { return { data, num }; }
        Span<MutT> SkipMut(usize len)                   mut { return { data + len, size - len }; }
        Span<MutT> HeadMut()                            mut { return { data + 1, size - 1 }; }
        Span<MutT> LastMut(usize num)                   mut { return { data + size - num, num }; }
        Span<MutT> TruncMut(usize len)                  mut { return { data, size - len }; }
        Span<MutT> InitMut()                            mut { return { data, size - 1 }; }
        Tuple<Ref<MutT>,  Span<MutT>> SplitFirstMut()       mut { return { data[0], HeadMut() }; }
        Tuple<Span<MutT>, Ref<MutT>>  SplitLastMut()        mut { return { InitMut(), data[size - 1] }; }
        Tuple<Span<MutT>, Span<MutT>> SplitAtMut(usize at)  mut { return { FirstMut(at), SkipMut(at) }; }
        Tuple<Span<MutT>, Ref<MutT>, Span<MutT>> PartitionAtMut(usize at) mut { return { FirstMut(at), data[at], SkipMut(at + 1) }; }
        Tuple<Span<MutT>, Span<MutT>> SplitOnMut(Predicate<T> auto&& pred) mut {
            const usize i = FindIf(pred);
            return i == USIZE_MAX ? Tuple { *this, Span<MutT>::Empty() } : SplitAtMut(i);
        }
        Tuple<Span<MutT>, Span<MutT>> RevSplitOnMut(Predicate<T> auto&& pred) mut {
            const usize i = RevFindIf(pred);
            return i == USIZE_MAX ? Tuple { Span<MutT>::Empty(), *this } : SplitAtMut(i);
        }
        Span<const T> Subspan(usize start)              const { return { data + start, size - start }; }
        Span<const T> Subspan(usize start, usize count) const { return { data + start, count }; }
        Span<const T> First(usize num)                  const { return { data, num }; }
        Span<const T> Skip(usize len)                   const { return { data + len, size - len }; }
        Span<const T> Head()                            const { return { data + 1, size - 1 }; }
        Span<const T> Last(usize num)                   const { return { data + size - num, num }; }
        Span<const T> Trunc(usize len)                  const { return { data, size - len }; }
        Span<const T> Init()                            const { return { data, size - 1 }; }
        Tuple<Ref<const T>,  Span<const T>> SplitFirst()       const { return { data[0], Head() }; }
        Tuple<Span<const T>, Ref<const T>>  SplitLast()        const { return { Init(), data[size - 1] }; }
        Tuple<Span<const T>, Span<const T>> SplitAt(usize at)  const { return { First(at), Skip(at) }; }
        Tuple<Span<const T>, Ref<const T>, Span<const T>> PartitionAt(usize at) const { return { First(at), data[at], Skip(at + 1) }; }
        Tuple<Span<const T>, Span<const T>> SplitOn(Predicate<T> auto&& pred) const {
            const usize i = FindIf(pred);
            return i == USIZE_MAX ? Tuple { *this, Empty() } : SplitAt(i);
        }
        Tuple<Span<const T>, Span<const T>> RevSplitOn(Predicate<T> auto&& pred) const {
            const usize i = RevFindIf(pred);
            return i == USIZE_MAX ? Tuple { Empty(), *this } : SplitAt(i);
        }

        // Option<FixedSpan<T, N>> FirstChunk<usize N>()
        // Tuple<FixedSpan<T, N>, Span> SplitFirstChunk<usize N>()
        // Option<FixedSpan<T, N>> LastChunk<usize N>()
        // Tuple<Span, FixedSpan<T, N>> SplitLastChunk<usize N>()

        // WindowIter                               Windows(usize len)
        // FixedWindowIter                          WindowsFixed(usize len)
        // ChunkIter                                Chunks(usize chunk)
        // RevChunkIter                             RevChunks(usize chunk)
        // ChunkExactIter                           ChunksExact(usize chunk)
        // RevChunkExactIter                        RevChunksExact(usize chunk)
        // FixedChunkExactIter                      FixedChunksExact<usize N>()
        // Span<Array<RemConst<T>, N>>              AsChunksExact<usize N>()
        // Tuple<Span<Array<RemConst<T>, N>>, Span> AsChunks<usize N>()
        // Tuple<Span, Span<Array<RemConst<T>, N>>> AsRevChunks<usize N>()
        //
        // ChunkByIter           ChunkBy(Fn<bool, T&, T&> pred)
        // SplitIter             Split(const T& sep)
        // SplitIfIter           SplitIf(Fn<bool, T&> pred)
        // SplitIfInclIter       SplitIfIncl(Fn<bool, T&> pred)
        // RevSplitIter          RevSplit(const T& sep)
        // RevSplitIfIter        RevSplitIf(Fn<bool, T&> pred)
        // SplitAtmostIfIter     SplitIfAtmost(usize maxLen, Fn<bool, T&, T&> pred)
        // RevSplitAtmostIfIter  RevSplitIfAtmost(usize maxLen, Fn<bool, T&, T&> pred)

        Span& Advance(usize num)    { data += num;    return *this; }
        Span& Shorten(usize amount) { size -= amount; return *this; }
        Span<MutT> TakeFirstMut(usize num) { Span<MutT> first    = FirstMut(num); data += num; size -= num; return first; }
        Span<MutT> TakeLastMut(usize num)  { Span<MutT> last     = LastMut(num);  size -= num;              return last; }
        MutT&      TakeFirstMut()          { MutT& first         = FirstMut();    ++data; --size;           return first; }
        MutT&      TakeLastMut()           { MutT& last          = LastMut();     --size;                   return last; }
        Span<MutT> TakeAfterMut(usize i)   { Span<MutT> after    = SkipMut(i);    size = i;                 return after; }
        Span<const T> TakeFirst(usize num) { Span<const T> first = First(num);    data += num; size -= num; return first; }
        Span<const T> TakeLast(usize num)  { Span<const T> last  = Last(num);     size -= num;              return last; }
        const T&      TakeFirst()          { const T& first      = First();       ++data; --size;           return first; }
        const T&      TakeLast()           { const T& last       = Last();        --size;                   return last; }
        Span<const T> TakeAfter(usize i)   { Span<const T> after = Skip(i);       size = i;                 return after; }

        template <usize N> Array<Ref<T>, N> GetMany(const Array<usize, N>& indices) {
            Array<Ref<T>, N> values;
            for (usize i = 0; i < N; ++i) { values[i].SetRef(data[indices[i]]); }
            return values;
        }

        void Swap(usize i, usize j) mut { std::swap(data[i], data[j]); }
        void Reverse() {
            for (usize i = 0, j = size - 1; i < j; ++i, --j) Swap(i, j);
        }

        bool RefEquals(Span other) const { return data == other.data && size == other.size; }
        bool ContainsBuffer(Span buf) const { return buf.data >= data && data + size >= buf.data + size; }
        bool OverlapsBuffer(Span buf) const {
            const T* end = data + size, * bufEnd = buf.data + buf.size;
            return end >= buf.data && bufEnd >= data;
        }
        bool Equals(Span other) const { return EqualsBy(other, Cmp::Equality {}); }
        bool EqualsBy(Span other, EqualPred<T> auto&& eq) const {
            if (size != other.size) return false;
            for (usize i = 0; i < size; ++i) if (!eq(data[i], other[i])) return false; return true;
        }
        bool EqualsByKey(Span other, FnArgs<const T&> auto&& keyf) const;
        bool operator==(Span other) const { return Equals(other); }

        bool All(Predicate<T> auto&& pred = Combinate::Identity {}) const {
            for (const T& x : *this) { if (!pred(x)) return false; } return true;
        }
        bool Any(Predicate<T> auto&& pred = Combinate::Identity {}) const {
            for (const T& x : *this) { if (pred(x)) return true; } return false;
        }
        Comparison Cmp(Span other) const { return CmpBy(other, Cmp::Compare {}); }
        Comparison CmpBy(Span other, Comparator<T> auto&& cmp) const {
            for (usize i = 0; i < std::min(size, other.size); ++i) {
                const Comparison c = cmp(data[i], other[i]);
                if (c != Cmp::EQUAL) return c;
            }
            return Cmp::Between(size, other.size);
        }
        Comparison CmpByKey(Span other, FnArgs<const T&> auto&& keyf) const;
        Comparison CmpSized(Span other) const { return CmpSizedBy(other, Cmp::Compare {}); }
        Comparison CmpSizedBy(Span other, Comparator<T> auto&& cmp) const {
            return other.size == size ? CmpBy(other, cmp) : Cmp::Between(size, other.size);
        }
        Comparison CmpSizedByKey(Span other, FnArgs<const T&> auto&& keyf) const;
        Comparison operator<=>(Span other) const { return Cmp(other); }

        bool StartsWith(Span prefix) const { return size >= prefix.size && First(prefix.size) == prefix; }
        bool EndsWith  (Span suffix) const { return size >= suffix.size && Last(suffix.size)  == suffix; }
        Option<Span> RemovePrefix(Span prefix) const { return StartsWith(prefix) ? Options::Some(Skip(prefix.size)) : nullptr; }
        Option<Span> RemoveSuffix(Span suffix) const { return EndsWith(suffix)   ? Options::Some(Trunc(suffix.size)) : nullptr; }


        usize Find   (const T& target) const { return FindIf   (Cmp::Equals { target }); }
        usize RevFind(const T& target) const { return RevFindIf(Cmp::Equals { target }); }
        usize FindIf(Predicate<T> auto&& pred) const {
            for (usize i = 0; i < size; ++i) if (pred(data[i])) return i; return USIZE_MAX;
        }
        usize RevFindIf(Predicate<T> auto&& pred) const {
            for (usize i = size; i --> 0; )  if (pred(data[i])) return i; return USIZE_MAX;
        }
        bool Contains     (const T& target) const { return Find(target)    != USIZE_MAX; }
        bool RevContains  (const T& target) const { return RevFind(target) != USIZE_MAX; }
        bool ContainsIf   (Predicate<T> auto&& pred) const { return FindIf(pred)    != USIZE_MAX; }
        bool RevContainsIf(Predicate<T> auto&& pred) const { return RevFindIf(pred) != USIZE_MAX; }

        usize Unaddress      (const T* addr) const { return addr - data; }
        bool  ContainsAddress(const T* addr) const { return data <= addr && addr < data + size; }
        // usize UnaddressSpan      (Span addr) const { return ; }
        // bool  ContainsAddressSpan(const T* addr) const { return ; }

        // assumes ascending order
        Tuple<bool, usize> BinarySearch(const T& target) const { return BinarySearchWith(Cmp::ComparedTo { target }); }
        // assumes ascending order
        Tuple<bool, usize> BinarySearchWith(Fn<Comparison, const T&> auto&& cmp) const;
        Tuple<bool, usize> BinarySearchByKey(const T& target, FnArgs<const T&> auto&& keyf) const;
        usize BinaryPartitionPoint(const T& target) const { return BinaryPartitionPointBy(Cmp::LessThanWith { target }); }
        usize BinaryPartitionPointBy(Predicate<T> auto&& left) const;
        usize BinaryPartitionPointByKey(const T& target, FnArgs<const T&> auto&& keyf) const;
        usize LowerBound(const T& target) const { return LowerBoundBy(Cmp::ComparedTo { target }); }
        usize LowerBoundBy(Fn<Comparison, const T&> auto&& cmp) const { const auto [found, x] = BinarySearchWith(cmp); return found ? x : std::min<usize>(x, 1) - 1; }
        usize LowerBoundByKey(const T& target, FnArgs<const T&> auto&& keyf) const;
        usize UpperBound(const T& target) const { return UpperBoundBy(Cmp::LessThanWith { target }); }
        usize UpperBoundBy(Fn<Comparison, const T&> auto&& cmp) const;
        usize UpperBoundByKey(const T& target, FnArgs<const T&> auto&& keyf) const;
        // Range<usize> EqualRange(const T& target) const { return LowerBoundBy(Operators::LessThanWith { target }); }
        // Range<usize> EqualRangeBy(Predicate<T> auto&& left) const;
        // Range<usize> EqualRangeByKey(FnArgs<const T&> auto&& keyf) const;

        bool BinaryContains(const T& target) const { const auto [found, _] = BinarySearch(target); return found; }
        bool BinaryContainsKey  (FnArgs<const T&> auto&& keyf, auto&& targetKey) const { const auto [found, _] = BinarySearchByKey(keyf(targetKey)); return found; }
        bool BinaryContainsByKey(FnArgs<const T&> auto&& keyf, const T& target)  const { const auto [found, _] = BinarySearchByKey(keyf(target));    return found; }

        void Sort() mut { return SortBy(Cmp::LessThan {}); }
        void SortBy(Comparator<T> auto&& cmp) mut;
        void SortByKey(FnArgs<const T&> auto&& keyf) mut;

        // void SortStable() TODO i cant be bothered to do this
        // void SortStableBy(Fn<bool, const T&, const T&> auto&& cmp)
        // void SortStableByKey(auto&& keyf)

        bool IsSorted() const { return IsSortedBy(Cmp::LessThan {}); }
        bool IsSortedBy(Comparator<T> auto&& cmp) const;
        bool IsSortedByKey(FnArgs<const T&> auto&& keyf) const;

        usize SortedPartitionPoint(usize idx) mut { return SortedPartitionPointBy(idx, Cmp::LessThan {}); }
        usize SortedPartitionPointBy(usize idx, Comparator<T> auto&& cmp) mut;
        usize SortedPartitionPointByKey(usize idx, FnArgs<const T&> auto&& keyf) mut;

        Tuple<Span, Ref<T>, Span> SortedPartition(usize idx) mut { return SortedPartitionBy(idx, Cmp::LessThan {}); }
        Tuple<Span, Ref<T>, Span> SortedPartitionBy(usize idx, Comparator<T> auto&& cmp) mut {
            const usize i = SortedPartitionPointBy(idx, cmp);
            return PartitionAt(i);
        }
        Tuple<Span, Ref<T>, Span> SortedPartitionByKey(usize idx, FnArgs<const T&> auto&& keyf) mut;

        usize PartitionDupPoint() mut { return PartitionDupPointBy(Cmp::Equality {}); }
        usize PartitionDupPointBy(EqualPred<T> auto&& eq) mut;
        Tuple<Span, Span> PartitionDup() mut { return PartitionDupBy(Cmp::Equality {}); }
        Tuple<Span, Span> PartitionDupBy(EqualPred<T> auto&& eq) mut {
            const usize i = PartitionDupPointBy(eq);
            return SplitAt(i);
        }

        void RotateLeft(usize num) mut;
        void RotateRight(usize num) mut;
        void RotateSigned(isize off)  mut { return off == 0 ? void() : off < 0 ? RotateLeft(off) : RotateRight(off); }
        void RotateSigned(WrappingIndex off)  mut { return RotateRight(off(size)); }
        void RotateSignedWrapped(isize off)  mut { return RotateRight((off % (isize)size + (isize)size) % (isize)size); }

        void Fill(const T& value) mut { for (T& t : *this) t = value; }
        void FillWith(Fn<T> auto&& factory) mut { for (T& t : *this) t = factory(); }
        void FillDefault() { return FillWith(Combinate::Constructor<T> {}); }
        void CloneFrom(Span<const T> span) mut { Memory::RangeCopy(data, span.data, span.count); }
        void MoveFrom(Span span) mut { Memory::RangeMove(data, span.data, span.count); }
        // void CopyFromSelf(IntegerRange, usize dest)
        void SwapWith(Span span) mut { Memory::RangeSwap(data, span.data, span.count); }

        // Tuple<Span, Span<AddConstIf<SimdT, T>>, Span> AsSimd() const;

        /* Span<AddConstIf<ArrayElement<T>, T>> */ auto AsFlattened() const requires IsRawArray<T> {
            return Span { (AddConstIf<ArrayElement<T>, T>*)data, size * ArrayLength<T>() };
        }

        ArrayBox<MutT> CollectToBox() const {
            T* arr = Memory::AllocateArrayUninit<T>(size);
            for (usize i = 0; i < size; ++i) Memory::ConstructCopyAt(&arr[i], data[i]);
            return ArrayBox<MutT>::Own(arr, size);
        }
        ArrayBox<MutT> MoveToBox() {
            T* arr = Memory::AllocateArrayUninit<T>(size);
            for (usize i = 0; i < size; ++i) Memory::ConstructMoveAt(&arr[i], std::move(data[i]));
            return ArrayBox<MutT>::Own(arr, size);
        }
        Vec<MutT> CollectToVec() const;
        Vec<MutT> MoveToVec() mut;
        Vec<MutT> Repeat(usize num) const;
        // Vec<ConcatResult<T>> Concat() const requires CanConcat<T>;
        // Vec<ConcatResult<T>> Join(const auto& sep) const requires CanConcat<T> && CanConcat<T, decltype(sep)>;

        template <class U>
        Tuple<Span, Span<AddConstIf<U, T>>, Span> TransmuteAligned() const requires SameAs<const T, const byte> {
            const usize p = (usize)data;
            const usize prefix = -p & (alignof(U) - 1), mid = (size - prefix) / sizeof(U), suffix = size - prefix - mid * sizeof(U);
            return Tuple {
                Span { (T*)p,                               prefix }, // super cool magic, ex: 0b010 + 0b110 to sum to alignof(T)
                Span { (AddConstIf<U, T>*)(p + prefix),     mid },
                Span { (T*)(p + size * sizeof(U) - suffix), suffix }
            };
        }

        template <class U> requires (sizeof(T) % sizeof(U) == 0) || (sizeof(U) % sizeof(T) == 0)
        Span<AddConstIf<U, T>> Transmute() const { return { Memory::TransmutePtr<AddConstIf<U, T>>(data), size * sizeof(T) / sizeof(U) }; }

        Span<MutT>      AsMut()   const { return { Memory::AsMutPtr(data),   size }; }
        Span<const T>   AsConst() const { return { Memory::AsConstPtr(data), size }; }

        operator Span<const T>() const { return AsConst(); }
    };

    template <class T> requires ContinuousCollectionAny<T>
    Span(T& collection) -> Span<AddConstIf<CollectionItem<T>, T>>;

    using Bytes = Span<const byte>;
    using BytesMut = Span<byte>;

    namespace Spans {
        template <class T>
        Span<T> FromBuffer(T* data, usize size) { return Span<T>::FromBuffer(data, size); }
        template <class T>
        Span<const T> FromIList(IList<T> ilist) { return Span<const T>::FromBuffer(ilist.begin(), ilist.size()); }
        template <class T, usize N>
        Span<T> Vals(T (&arr)[N])  { return Span<T>::FromArray(arr); }
        template <class T, usize N>
        Span<T> Vals(T (&&arr)[N]) { return Span<T>::FromArray(arr); }
    }

    template <class T>
    struct BufferIterator : IContinuousIterator<T, BufferIterator<T>> {
        friend IIterator<T, BufferIterator>;
        friend IContinuousIterator<T, BufferIterator>;

        RemRef<T>* iter;
        RemRef<T>* end;

        BufferIterator(RemRef<T>* beg, RemRef<T>* end) : iter(beg), end(end) {}

    protected:
        T CurrentImpl() const {
            if constexpr (!IsRef<T>) return std::move(*iter);
            else return *iter;
        }
        void AdvanceImpl() { ++iter; }
        bool CanNextImpl() const { return iter != end; }
    public:
        bool operator==(const BufferIterator& it) const { return iter == it.iter; }
    };

    template <class T, class Super>
    Span<const byte> IContinuousCollection<T, Super>::AsBytes()  const { return AsSpan().AsBytes(); }
    template <class T, class Super>
    Span<byte>       IContinuousCollection<T, Super>::AsBytesMut() mut { return AsSpan().AsBytesMut(); }

    template <class T, class A> Span<const T> Box<T, A>::AsSpan() const { return data ? Span<const T>::Single(*data) : nullptr; }
    template <class T, class A> Span<T>       Box<T, A>::AsSpanMut()    { return data ? Span<T>::Single(*data)       : nullptr; }
}
