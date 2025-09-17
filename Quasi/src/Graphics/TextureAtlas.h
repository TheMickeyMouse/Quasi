#pragma once
#include "Image.h"
#include "GLs/Texture.h"

namespace Quasi::Graphics {
    class TextureAtlas {
        Texture2D fullTexture;
        Vec<Math::uRect2D> spritesheet;
        HashMap<String, u32> spriteLookup;
    public:
        TextureAtlas() = default;
        TextureAtlas(Vec<ImageView> sprites);
        TextureAtlas(Vec<ImageView> sprites, Span<const Str> spriteNames);
    private:
        void PackSprites(Span<ImageView> sprites);
    public:
        Texture2D& GetTexture() { return fullTexture; }
        const Texture2D& GetTexture() const { return fullTexture; }

        Math::uRect2D GetPx(Str name) const;
        Math::uRect2D GetPx(u32 id)   const;
        Math::fRect2D GetUV(Str name) const;
        Math::fRect2D GetUV(u32 id)   const;

        Math::fRect2D operator[](Str name) const { return GetUV(name); }
        Math::fRect2D operator[](u32 id)   const { return GetUV(id);   }
    };
}
