#pragma once

#include "Utils/Math/Vector.h"
#include "Utils/Math/Rect.h"
#include "Utils/Math/Color.h"

namespace Quasi::Graphics {
    struct ImageView;

    struct Image {
        Box<u8> imageData = nullptr;
        u32 width = 0, height = 0;

        Image() = default;
    private:
        Image(u8* imageData, u32 w, u32 h) : imageData(Box<u8>::Own(imageData)), width(w), height(h) {}
    public:
        static Image New(u32 w, u32 h);
        static Image SolidColor(const Math::uColor& color, u32 w, u32 h);
        static Image FromData(u8* imageData, u32 w, u32 h) { return { imageData, w, h }; }
        static Image CopyData(ImageView view);
        static Image LoadPNGBytes(Bytes pngbytes);
        static Image LoadPNG(CStr fname);

        ImageView AsView() const;
        ImageView SubImage(u32 x, u32 y, u32 w, u32 h) const;
        ImageView SubImage(const Math::uRect2D& subrect) const;

        bool InBounds(const Math::iv2& coord) const;
        const Math::uColor& GetPx(const Math::uv2& coord) const;
        Math::uColor& GetPx(const Math::uv2& coord);
        OptRef<const Math::uColor> TryGetPx(const Math::iv2& coord) const;
        OptRef<Math::uColor> TryGetPx(const Math::iv2& coord);

        Span<const Math::uColor> GetRow(u32 y) const;
        Span<Math::uColor> GetRow(u32 y);

        const Math::uColor& operator[](const Math::uv2& coord) const { return GetPx(coord); }
        Math::uColor& operator[](const Math::uv2& coord) { return GetPx(coord); }
        Span<const Math::uColor> operator[](u32 y) const { return GetRow(y); }
        Span<Math::uColor> operator[](u32 y) { return GetRow(y); }

        u8* ReleaseData() { return imageData.Release(); }
        const u8* Data() const { return imageData.Data(); }
        u8* Data() { return imageData.Data(); }
        const Math::uColor* PixelData() const;
        Math::uColor* PixelData();
        Math::uv2 Size() const { return { width, height }; }

        void BlitImage(const Math::uv2& dest, const ImageView& image);

        void ExportPNG(Str fdest);
    };

    struct ImageView {
        const u8* data = nullptr;
        u32 width = 0, height = 0;
        // stride is the amount to advance to the next row
        u32 stride = 0;

        ImageView() = default;
    private:
        ImageView(const u8* data, u32 w, u32 h, u32 stride) : data(data), width(w), height(h), stride(stride) {}
    public:
        ImageView(const Image& img) : data(img.imageData.Data()), width(img.width), height(img.height), stride(img.width) {}

        static ImageView FromData(const u8* imageData, u32 w, u32 h);
        static ImageView FromData(const u8* imageData, u32 w, u32 h, u32 stride);

        ImageView SubImage(u32 x, u32 y, u32 w, u32 h) const;
        ImageView SubImage(const Math::uRect2D& subrect) const;

        bool InBounds(const Math::iv2& coord) const;
        const Math::uColor& GetPx(const Math::uv2& coord) const;
        OptRef<const Math::uColor> TryGetPx(const Math::iv2& coord) const;

        Span<const Math::uColor> GetRow(u32 y) const;

        const Math::uColor& operator[](const Math::uv2& coord) const { return GetPx(coord); }
        Span<const Math::uColor> operator[](u32 y) const { return GetRow(y); }

        const u8* Data() const { return data; }
        const Math::uColor* PixelData() const;
        Math::uv2 Size() const { return { width, height }; }

        void ExportPNG(Str fdest);
    };
}
