#include "Image.h"

#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"
#include "Utils/CStr.h"

namespace Quasi::Graphics {
    Image Image::New(u32 w, u32 h) {
        return { CBox<u8>::Own(Memory::AllocateArrayUninit<u8>(w * h * 4)), w, h };
    }

    Image Image::SolidColor(const Math::uColor& color, u32 w, u32 h) {
        Image solidColor = New(w, h);
        Memory::RangeSet(solidColor.PixelData(), color, w * h);
        return solidColor;
    }

    Image Image::CopyData(ImageView view) {
        u8* data = Memory::AllocateArrayUninit<u8>(view.width * view.height * 4);
        for (u32 y = 0; y < view.height; y++) {
            Memory::MemCopyNoOverlap(&data[y * view.width], &view.data[y * view.stride], view.width * 4);
        }
        return { data, view.width, view.height };
    }

    Image Image::LoadPNGBytes(Bytes pngbytes) {
        int w, h, BPPixel;
        stbi_set_flip_vertically_on_load(1);
        u8* localTexture = stbi_load_from_memory(pngbytes.Data(), (int)pngbytes.Length(), &w, &h, &BPPixel, 4);
        return FromData(localTexture, (u32)w, (u32)h);
    }

    Image Image::LoadPNG(CStr fname) {
        int w, h, BPPixel;
        stbi_set_flip_vertically_on_load(1);
        u8* localTexture = stbi_load(fname.Data(), &w, &h, &BPPixel, 4);
        return FromData(localTexture, (u32)w, (u32)h);
    }

    ImageView Image::AsView() const {
        return { *this };
    }

    ImageView Image::SubImage(u32 x, u32 y, u32 w, u32 h) const {
        return ImageView::FromData(Data() + 4 * (x + y * width), w, h, width);
    }

    ImageView Image::SubImage(const Math::uRect2D& subrect) const {
        return ImageView::FromData(Data() + 4 * (subrect.min.x + subrect.min.y * width), subrect.Width(), subrect.Height(), width);
    }

    bool Image::InBounds(const Math::iv2& coord) const {
        return 0 < coord.x && coord.x < width && 0 < coord.y && coord.y < height;
    }

    const Math::uColor& Image::GetPx(const Math::uv2& coord) const {
        return PixelData()[coord.x + coord.y * width];
    }

    Math::uColor& Image::GetPx(const Math::uv2& coord) {
        return PixelData()[coord.x + coord.y * width];
    }

    OptRef<const Math::uColor> Image::TryGetPx(const Math::iv2& coord) const {
        return InBounds(coord) ? OptRefs::SomeRef(GetPx((Math::uv2)coord)) : nullptr;
    }

    OptRef<Math::uColor> Image::TryGetPx(const Math::iv2& coord) {
        return InBounds(coord) ? OptRefs::SomeRef(GetPx((Math::uv2)coord)) : nullptr;
    }

    Span<const Math::uColor> Image::GetRow(u32 y) const {
        return Spans::Slice(PixelData() + y * width, width);
    }

    Span<Math::uColor> Image::GetRow(u32 y) {
        return Spans::Slice(PixelData() + y * width, width);
    }

    const Math::uColor* Image::PixelData() const {
        return Memory::TransmutePtr<const Math::uColor>(imageData.Data());
    }
    Math::uColor* Image::PixelData() {
        return Memory::TransmutePtr<Math::uColor>(imageData.Data());
    }

    void Image::BlitImage(const Math::uv2& dest, const ImageView& image) {
        u8* begin = (u8*)&PixelData()[dest.x + dest.y * width];
        for (u32 y = 0; y < image.height; y++) {
            Memory::MemCopyNoOverlap(begin + y * width, &image.data[y * image.stride], image.width * 4);
        }
    }

    void Image::ExportPNG(Str fdest) {
        stbi_write_png(fdest.Data(), (int)width, (int)height, 4, imageData.Data(), 0);
    }

    ImageView ImageView::FromData(const u8* imageData, u32 w, u32 h) {
        return { imageData, w, h, w };
    }

    ImageView ImageView::FromData(const u8* imageData, u32 w, u32 h, u32 stride) {
        return { imageData, w, h, stride };
    }

    ImageView ImageView::SubImage(u32 x, u32 y, u32 w, u32 h) const {
        return { data + 4 * (x + y * stride), w, h, stride };
    }

    ImageView ImageView::SubImage(const Math::uRect2D& subrect) const {
        return { data + 4 * (subrect.min.x + subrect.min.y * stride), subrect.Width(), subrect.Height(), stride };
    }

    bool ImageView::InBounds(const Math::iv2& coord) const {
        return 0 < coord.x && coord.x < width && 0 < coord.y && coord.y < height;
    }

    const Math::uColor& ImageView::GetPx(const Math::uv2& coord) const {
        return PixelData()[coord.x + coord.y * stride];
    }

    OptRef<const Math::uColor> ImageView::TryGetPx(const Math::iv2& coord) const {
        return InBounds(coord) ? OptRefs::SomeRef(GetPx((Math::uv2)coord)) : nullptr;
    }

    Span<const Math::uColor> ImageView::GetRow(u32 y) const {
        return Spans::Slice(PixelData() + y * stride, width);
    }

    const Math::uColor* ImageView::PixelData() const {
        return Memory::TransmutePtr<const Math::uColor>(data);
    }

    void ImageView::ExportPNG(Str fdest) {
        stbi_write_png(fdest.Data(), (int)width, (int)height, 4, data, (int)stride * 4);
    }
}
