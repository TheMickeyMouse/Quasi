#pragma once
#include "Texture.h"
#include "Vector.h"

namespace Graphics {
    class TextureAtlas {
        private:
            Texture texture;
            Maths::uvec2 atlasSize;
            Maths::uvec2 subTextureSize;
        public:
            OPENGL_API explicit TextureAtlas(const std::string& filename, uint xSize, uint ySize);

            [[nodiscard]] Maths::uvec2 GetTextureSize()    const { return texture.GetSize(); }
            [[nodiscard]] Maths::uvec2 GetAtlasSize()      const { return atlasSize; }
            [[nodiscard]] Maths::uvec2 GetSubTextureSize() const { return subTextureSize; }

            Texture& GetTexture() { return texture; }
            [[nodiscard]] const Texture& GetTexture() const { return texture; }
    };
}
