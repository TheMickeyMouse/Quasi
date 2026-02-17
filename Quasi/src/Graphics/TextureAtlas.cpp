#include "TextureAtlas.h"
#include "Utils/Algorithm.h"

namespace Quasi::Graphics {
    TextureAtlas::TextureAtlas(Vec<ImageView> sprites) {
        sprites.SortByKey([] (const ImageView& sprite) { return sprite.height; });
        PackSprites(sprites);
    }

    TextureAtlas::TextureAtlas(Vec<ImageView> sprites, Span<const Str> spriteNames) {
        Vec<u32> indices = Vecs::Range<u32>(0, sprites.Length());
        indices.SortByKey([&] (u32 i) { return sprites[i].height; });
        Algorithm::ApplyPermutationInPlace(sprites.AsSpan(), indices.AsSpan());

        spriteLookup.Reserve(spriteNames.Length());
        for (usize i = 0; i < spriteNames.Length(); ++i) {
            spriteLookup.Insert(spriteNames[i], indices[i]);
        }

        PackSprites(sprites);
    }

    void TextureAtlas::PackSprites(Span<ImageView> sprites) {
        // https://www.david-colson.com/2020/03/10/exploring-rect-packing.html
        // we're rect packing now; using the simplest solution: naive row packing

        // first fit the first √N elements
        const int firstRow = (int)std::sqrt(sprites.Length());

        Math::iv2 pen;
        for (int i = 0; i < firstRow; i++) {
            spritesheet.Push(Math::iRect2D::FromSize(pen, sprites[i].Size()));
            pen.x += sprites[i].width;
        }
        const u32 width = pen.x;
        pen.y = sprites[0].height;

        // proceed to fit the next sprites, making sure to not exceed the first row
        for (int i = firstRow; i < sprites.Length(); i++) {
            pen.x = 0;
            const u32 initialHeight = sprites[i].height;
            while (pen.x < width) {
                spritesheet.Push(Math::iRect2D::FromSize(pen, sprites[i].Size()));
                pen.x += sprites[i].width;
            }
            pen.y += initialHeight;
        }

        Image atlas = Image::New(width, pen.y);

        for (usize i = 0; i < sprites.Length(); ++i) {
            atlas.BlitImage(spritesheet[i].min, sprites[i]);
        }

        fullTexture = Texture2D::New(atlas);
    }

    Math::iRect2D TextureAtlas::GetPx(Str name) const {
        const Option<u32> id = spriteLookup.Get(name).Copied();
        if (!id) return Math::iRect2D::Empty();
        return GetPx(*id);
    }

    Math::iRect2D TextureAtlas::GetPx(u32 id) const {
        return spritesheet[id];
    }

    Math::fRect2D TextureAtlas::GetUV(Str name) const {
        return fullTexture.Px2UV(GetPx(name));
    }

    Math::fRect2D TextureAtlas::GetUV(u32 id) const {
        return fullTexture.Px2UV(GetPx(id));
    }
}
