#pragma once
#include <string>
#include "opengl.h"

#include "NumTypes.h"
#include "Vector.h"

namespace Graphics {
    class Texture {
    private:
        glID rendererID = GL_NULL;
        Maths::uvec2 size;
        int BPPixel = 0; //stands for bits per pixel
        
        class GraphicsDevice* device = nullptr;
        uint textureSlot = 0;

        OPENGL_API void LoadTexture(const uchar* img, bool useLinear = true, int format = 0x1908, int alignment = 4 /* see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPixelStore.xhtml */);
     public:
        OPENGL_API Texture() = default;
        OPENGL_API explicit Texture(const uchar* raw, uint w, uint h, bool useLinear = true, int format = 0x1908 /* GL_RGBA */, int alignment = 4);
        OPENGL_API explicit Texture(const std::string& filePath, bool useLinear = true);
        OPENGL_API ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        OPENGL_API static void Transfer(Texture& dest, Texture&& from);
        Texture(Texture&& tex) noexcept { Transfer(*this, std::move(tex)); }
        Texture& operator=(Texture&& tex) noexcept { Transfer(*this, std::move(tex)); return *this; }

        OPENGL_API static Texture LoadPNGBytes(const uchar* png, int len, bool useLinear = true);

        OPENGL_API void Bind(uint slot, GraphicsDevice* gdevice);
        OPENGL_API void Unbind();
        OPENGL_API void Destroy();

        [[nodiscard]] int Slot() const { return (int)textureSlot; }

        [[nodiscard]] Maths::uvec2 GetSize() const { return size; }
    };
}
