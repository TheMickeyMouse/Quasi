#pragma once
#include "GLs/Texture.h"

namespace Quasi::Graphics {
    class TextureAtlas {
        Texture2D fullTexture;
        Vec<Math::fRect2D> spritesheet;
        HashMap<Str, u32> spriteLookup;
    public:
        TextureAtlas() = default;
        TextureAtlas(Vec<Math::fv2>&& imageSizes);
        TextureAtlas(Vec<Math::fRect2D>&&, Span<const Str> spriteNames);

        Texture2D& GetTexture();
        const Texture2D& GetTexture() const;

        const Math::fRect2D& Get(Str name) const;
        const Math::fRect2D& Get(u32 id) const;
    };
}
