#pragma once
#include "Memory.h"
#include "Numeric.h"
#include "Span.h"
#include "Math/Vector.h"

namespace Quasi {
    template <class T> struct GridView;

    template <class T>
    struct GridArray {
        using uv2 = Math::uv2;
    private:
        // x is stored continuously, y requires index increase of width
        T* data;
        u32 width, height;

        GridArray(T* data, u32 width, u32 height) : data(data), width(width), height(height) {}
    public:
        GridArray() : data(nullptr), width(0), height(0) {}
        ~GridArray() { Memory::RangeDestruct(data, width * height); Memory::FreeRaw(data); }

        GridArray(GridArray&& grid) noexcept : data(grid.data), width(grid.width), height(grid.height) { grid.PretendClear(); }
        GridArray& operator=(GridArray&& grid) noexcept { this->~GridArray(); data = grid.data; width = grid.width; height = grid.height; grid.PretendClear(); return *this; }
    private:
        void PretendClear() { data = nullptr; width = height = 0; }
    public:
        static GridArray Empty() { return {}; }

        static GridArray CopyFromData(const T* data, u32 width, u32 height) {
            GridArray g = WithSize(width, height);
            Memory::RangeCopyNoOverlap(g.Data(), data, g.Total());
            return g;
        }
        static GridArray MoveFromData(T* data, u32 width, u32 height) {
            GridArray g = WithSize(width, height);
            Memory::RangeMoveNoOverlap(g.Data(), data, g.Total());
            return g;
        }
        template <usize M, usize N> static GridArray New(T (&&arr)[M][N]) {
            return MoveFromData(&arr[0], N, M);
        }
        template <usize M, usize N> static GridArray New(const T (&arr)[M][N]) {
            return MoveFromData(&arr[0], N, M);
        }
        static GridArray Fold(Span<const T> elms, u32 width) {
            return CopyFromData(elms.Data(), width, elms.Length() / width);
        }
        static GridArray MoveFold(Span<T> movElms, u32 width) {
            return CopyFromData(movElms.Data(), width, movElms.Length() / width);
        }

        static GridArray WithSize(u32 width, u32 height) { return { AllocateBuffer(width, height), width, height }; }

        GridArray Clone() const { return Fold(AsSpan(), width); }
    public:
        static T* AllocateBuffer(u32 width, u32 height) { return Memory::AllocateArrayUninit<T>(width * height); }

        void Clear() { Memory::RangeDestruct(data, Total()); width = height = 0; }
        [[nodiscard]] T* Release() { T* const d = data; PretendClear(); return d; }

        const T* Data() const { return data; }
        T* Data() { return data; }
        u32 Width()    const { return width; }
        u32 Height()   const { return height; }
        uv2 Size()     const { return { width, height }; }
        u32 Total()    const { return width * height; }
        bool IsEmpty() const { return Total() == 0; }
        explicit operator bool() const { return !IsEmpty(); }

        Span<const T> AsSpan() const { return Spans::Slice(data, Total()); }
        Span<T>       AsSpan()       { return Spans::Slice(data, Total()); }

        bool InBounds(uv2 p) const { return p.x < width && p.y < height; }
        T& At(uv2 p) { return data[p.y * width + p.x]; }
        const T& At(uv2 p) const { return data[p.y * width + p.x]; }
        OptRef<T> TryAt(uv2 p) { return InBounds(p) ? OptRefs::SomeRef(At(p)) : nullptr; }
        OptRef<const T> TryAt(uv2 p) const { return InBounds(p) ? OptRefs::SomeRef(At(p)) : nullptr; }

        T& operator[](uv2 p) { return At(p); }
        const T& operator[](uv2 p) const { return At(p); }

        GridView<const T> AsView() const { return { data, width, height, width }; }
        GridView<T> AsView() { return { data, width, height, width }; }

        GridView<const T> SubGrid(uv2 p, uv2 size) const { return AsView().SubGrid(p, size); }
        GridView<T> SubGrid(uv2 p, uv2 size) { return AsView().SubGrid(p, size); }

        GridView<const T> Neighbors(uv2 p, u32 distance = 1) const { return AsView().Neighbors(p, distance); }
        GridView<T> Neighbors(uv2 p, u32 distance = 1) { return AsView().Neighbors(p, distance); }

        uv2 Unaddress(const T* ptr) const {
            const usize offset = ptr - data;
            const u32 x = offset % width, y = offset / width;
            return { x, y };
        }

        friend struct GridView<T>;
        friend struct GridView<const T>;
    };

    template <class T>
    struct GridView {
        using uv2 = Math::uv2;
    private:
        T* data = nullptr;
        u32 width = 0, height = 0, stride = 0;

        GridView(T* data, u32 w, u32 h, u32 stride) : data(data), width(w), height(h), stride(stride) {}
    public:
        GridView() = default;
        GridView(const GridArray<T>& g) : data(g.data), width(g.width), height(g.height), stride(g.width) {}

        static GridView FromData(T* data, u32 w, u32 h, u32 stride = 0) { return { data, w, h, stride }; }

        const T* Data() const { return data; }
        T* Data() { return data; }
        u32 Width()    const { return width; }
        u32 Height()   const { return height; }
        uv2 Size()     const { return { width, height }; }
        u32 Total()    const { return width * height; }
        bool IsEmpty() const { return Total() == 0; }
        explicit operator bool() const { return !IsEmpty(); }

        operator GridView<const T>() const { return { data, width, height, stride }; }

        bool InBounds(uv2 p) const { return p.x < width && p.y < height; }
        T& At(uv2 p) { return data[p.y * stride + p.x]; }
        const T& At(uv2 p) const { return data[p.y * stride + p.x]; }
        OptRef<T> TryAt(uv2 p) { return InBounds(p) ? At(p) : nullptr; }
        OptRef<const T> TryAt(uv2 p) const { return InBounds(p) ? At(p) : nullptr; }

        T& operator[](uv2 p) { return At(p); }
        const T& operator[](uv2 p) const { return At(p); }

        GridView SubGrid(uv2 p, uv2 size) const {
            return { (T*)&At(p), size.x, size.y, stride };
        }

        GridView Neighbors(uv2 p, u32 distance = 1) const {
            const u32 sx = u32s::SatSub(p.x, distance), ex = std::min(width, p.x + distance + 1);
            const u32 sy = u32s::SatSub(p.y, distance), ey = std::min(height, p.y + distance + 1);
            return SubGrid({ sx, sy }, { ex - sx, ey - sy });
        }

        uv2 Unaddress(const T* ptr) const {
            const usize offset = ptr - data;
            const u32 x = offset % stride, y = offset / stride;
            return { x, y };
        }

        template <class U> friend struct GridView;
        friend struct GridArray<RemConst<T>>;
    };
} // Quasi