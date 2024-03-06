#pragma once
#include <string>
#include <vector>

#include "GLObject.h"
#include "..\..\..\core.h"

#include "NumTypes.h"
#include "Vector.h"
#include "stdu/unique.h"

namespace Graphics {
    // * from glew.h or https://javagl.github.io/GLConstantsTranslator/GLConstantsTranslator.html
    enum class TextureFormat {
        RED = 0x1903,
        RED_GREEN = 0x8227,
        RGB = 0x1907,
        RGB_REVERSE = 0x80E0,
        BGR = RGB_REVERSE,
        RGBA = 0x1908,
        RGBA_REVERSE = 0x80E1,
        BGRA = RGBA_REVERSE
    };

    enum class TextureInternalFormat {
        RGBA_8  = 0x8058, RGBA_8I  = 0x8D8E, RGBA_8UI  = 0x8D7C,
        RGBA_16 = 0x805B, RGBA_16I = 0x8D88, RGBA_16UI = 0x8D76, RGBA_16F  = 0x881A,
                          RGBA_32I = 0x8D82, RGBA_32UI = 0x8D70, RGBA_32F  = 0x8814,

        RGB_8   = 0x8051, RGB_8I   = 0x8D8F, RGB_8UI   = 0x8D7D,
        RGB_16  = 0x8054, RGB_16I  = 0x8D89, RGB_16UI  = 0x8D77, RGB_16F   = 0x881B,
                          RGB_32I  = 0x8D83, RGB_32UI  = 0x8D71, RGB_32F   = 0x8815,

        R_8     = 0x8229, R_8I     = 0x8231, R_8UI     = 0x8232,
        R_16    = 0x822A, R_16I    = 0x8233, R_16UI    = 0x8234, R_16F     = 0x822D,
                          R_32I    = 0x8235, R_32UI    = 0x8236, R_32F     = 0x822E,

        RG_8    = 0x822B, RG_8I    = 0x8237, RG_8UI    = 0x8238,
        RG_16   = 0x822C, RG_16I   = 0x8239, RG_16UI   = 0x823A, RG_16F    = 0x822F,
                          RG_32I   = 0x823B, RG_32UI   = 0x823C, RG_32F    = 0x8230,

        R8_SNORM  = 0x8F94, RG8_SNORM  = 0x8F95, RGB8_SNORM  = 0x8F96, RGBA8_SNORM  = 0x8F97,
        R16_SNORM = 0x8F98, RG16_SNORM = 0x8F99, RGB16_SNORM = 0x8F9A, RGBA16_SNORM = 0x8F9B,

        SRGB_8 = 0x8C41, SRGBA_8 = 0x8C43,

        RGBA_10_2 = 0x8059, RGBA_10_2UI = 0x906F,
        RGB_11_11_10F = 0x8C3A,
        RGB_9E5 = 0x8C3D,

        RGTC_COMPRESSED_RED = 0x8DBB, RGTC_COMPRESSED_SIGNED_RED = 0x8DBC,
        RGTC_COMPRESSED_RG  = 0x8DBD, RGTC_COMPRESSED_SIGNED_RG  = 0x8DBE,

        DEPTH_32F = 0x8CAC, DEPTH_32 = 0x81A7, DEPTH_24 = 0x81A6, DEPTH_16 = 0x81A5, DEPTH = 0x1902,
        DEPTH_32F_STENCIL_8 = 0x8CAD,          DEPTH_24_STENCIL_8 = 0x88F0
    };

    struct TextureHandler : GLObjectHandler<TextureHandler> {
        OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };

    struct TextureSlotHandler {
        constexpr int operator()() const { return -1; }
        OPENGL_API void operator()(int slot) const;
    };

    class Texture : public GLObject<TextureHandler> {
    public:
        inline static int SlotCount = -1;
        inline static std::vector<Texture*> Slots {};
        OPENGL_API static void Init();
    private:
        Maths::uvec2 size;
        int BPPixel = 0; //stands for bits per pixel'
        using slot_t = stdu::unique<int, TextureSlotHandler>;
        slot_t textureSlot = -1;

        OPENGL_API void LoadTexture(const uchar* img, bool useLinear = true,
            TextureFormat format = TextureFormat::RGBA,
            TextureInternalFormat iformat = TextureInternalFormat::RGBA_8,
            int alignment = 4 /* see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPixelStore.xhtml */);
     public:
        Texture() = default;
        OPENGL_API explicit Texture(const uchar* raw, uint w, uint h, bool useLinear = true,
            TextureFormat format = TextureFormat::RGBA,
            TextureInternalFormat iformat = TextureInternalFormat::RGBA_8,
            int alignment = 4);
        OPENGL_API explicit Texture(const std::string& filePath, bool useLinear = true);

        OPENGL_API static Texture LoadPNGBytes(const uchar* png, int len, bool useLinear = true);

        OPENGL_API void Activate(int slot = -1);
        OPENGL_API void Deactivate();
        OPENGL_API static void DeactivateAll();
        OPENGL_API static int FindEmptySlot();

        OPENGL_API void SetSubTexture(const uchar* data, Maths::rect2u rect, TextureFormat format = TextureFormat::RGBA);

        [[nodiscard]] int Slot() const { return *textureSlot; }
        [[nodiscard]] float Slotf() const { return (float)*textureSlot; }

        [[nodiscard]] Maths::uvec2 GetSize() const { return size; }

        friend class FrameBuffer;
    };
}
