#pragma once
#include <string>
#include <vector>

#include "GLObject.h"
#include <core.h>
#include <variant>

#include "GLTypeID.h"
#include "NumTypes.h"
#include "TextureConstants.h"
#include "Rect.h"
#include "stdu/unique.h"

namespace Graphics {
    struct TextureHandler : GLObjectHandler<TextureHandler> {
        TextureTarget target;

        [[nodiscard]] OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };

    struct TextureSlotHandler {
        constexpr int operator()() const { return -1; }
        OPENGL_API void operator()(int slot) const;
    };

    struct TextureParamPair {
        TextureParamName pname;
        std::variant<int, float, const int*, const float*> val;

        template <class E> requires std::is_enum_v<E>
        TextureParamPair(TextureParamName name, E val) : pname(name), val((int)val) {}
        TextureParamPair(TextureParamName name, int val) : pname(name), val(val) {}
        TextureParamPair(TextureParamName name, float val) : pname(name), val(val) {}
        TextureParamPair(TextureParamName name, const int* val) : pname(name), val(val) {}
        TextureParamPair(TextureParamName name, const float* val) : pname(name), val(val) {}

        static TextureParamPair NearestSample() { return { TextureParamName::XT_SAMPLE_FILTER, TextureSample::NEAREST }; }
        static TextureParamPair LinearSample()  { return { TextureParamName::XT_SAMPLE_FILTER, TextureSample::LINEAR }; }
        static TextureParamPair ClampedBorder()   { return { TextureParamName::XT_WRAPPING, TextureBorder::CLAMP_TO_EDGE }; }
        static TextureParamPair ColoredBorder()   { return { TextureParamName::XT_WRAPPING, TextureBorder::CLAMP_TO_BORDER }; }
        static TextureParamPair MirroredBorder()  { return { TextureParamName::XT_WRAPPING, TextureBorder::MIRRORED_REPEAT }; }
        static TextureParamPair RepeatingBorder() { return { TextureParamName::XT_WRAPPING, TextureBorder::REPEAT }; }

        template <class T> [[nodiscard]] bool is() const { return std::holds_alternative<T>(val); }
        template <class T> [[nodiscard]] T as() const { return std::get<T>(val); }
    };

    using TextureParameters = std::initializer_list<TextureParamPair>;

    struct TextureLoadParams {
        TextureFormat format = TextureFormat::RGBA;
        TextureIFormat internalformat = TextureIFormat::RGBA_8;
        GLTypeID type = GLTypeID::UBYTE;
        int level = 0;
    };

    struct TextureInitParams {
        TextureLoadParams load = {};
        TextureTarget target = TextureTarget::TEXTURE_2D;
        TextureParameters params;
    };

    struct STBIImageHandler {
        OPENGL_API void operator()(void* dat);
    };
    using STBIImage = std::unique_ptr<void, STBIImageHandler>;

    class Texture : public GLObject<TextureHandler> {
    public:
        inline static int SlotCount = -1;
        inline static std::vector<Texture*> Slots {};
        OPENGL_API static void Init();
    private:
        using slot_t = stdu::unique<int, TextureSlotHandler>;

        Maths::uvec3 size;
        int BPPixel = 0; //stands for bits per pixel'
        slot_t textureSlot = -1;

        OPENGL_API void DefaultParams() const;

        OPENGL_API void LoadTexture(const void* img, const TextureInitParams& init = {});
    public:
        Texture() = default;
        OPENGL_API Texture(stdu::empty);
        OPENGL_API explicit Texture(const void* raw, const Maths::uvec3& size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_3D });
        explicit Texture(const void* raw, const Maths::uvec2& size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_2D }) : Texture(raw, size.with_z(0), init) {}
        explicit Texture(const void* raw, uint size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_1D }) : Texture(raw, { size, 0, 0 }, init) {}

        OPENGL_API static Texture LoadPNGBytes(stdu::byte_span datapng, const TextureInitParams& init = {});
        OPENGL_API static Texture LoadPNG(const std::string& fname, const TextureInitParams& init = {});

        OPENGL_API static Texture LoadCubemapPNG(std::initializer_list<std::string> faces /* in order: rludfb */, const TextureInitParams& init = {});

        OPENGL_API void Activate(int slot = -1);
        OPENGL_API void Deactivate();
        OPENGL_API static void DeactivateAll();
        OPENGL_API static int FindEmptySlot();

        OPENGL_API void SetParam(TextureParamName param, float val) const;
        OPENGL_API void SetParam(TextureParamName param, int val) const;
        OPENGL_API void SetParam(TextureParamName param, const float* vals) const;
        OPENGL_API void SetParam(TextureParamName param, const int* vals) const;
        template <class E> requires std::is_enum_v<E>
        void SetParam(TextureParamName param, E val) const { SetParam(param, (int)val); }

        OPENGL_API void SetParams(TextureParameters params);

        OPENGL_API void SetSample(TextureSample s) const;
        OPENGL_API void SetWrapping(TextureBorder b) const;

        OPENGL_API void SetSubTexture(const void* data, const Maths::rect3u& rect, const TextureLoadParams& params = {});
        void SetSubTexture(const void* data, const Maths::rect2u& rect, const TextureLoadParams& params = {}) {
            SetSubTexture(data, Maths::rect3u { rect.min.with_z(0), rect.max.with_z(0) }, params);
        }
        void SetSubTexture(const uchar* data, const Maths::urange& rect, const TextureLoadParams& params = {}) {
            SetSubTexture(data, Maths::rect3u { { rect.min, 0, 0 }, { rect.max, 0, 0 } }, params);
        }

        OPENGL_API void TexImage(const void* data, const Maths::uvec3& dim, const TextureLoadParams& params = {}, TextureTarget overrideTarget = TextureTarget::NONE);
        void TexImage(const void* data, uint width, const TextureLoadParams& params = {}) {
            TexImage(data, { width, 0, 0 }, params);
        }
        void TexImage(const void* data, const Maths::uvec2& dim, const TextureLoadParams& params = {}) {
            TexImage(data, dim.with_z(0), params);
        }

        OPENGL_API static void SetPixelStore(PixelStoreParam param, int val);

        [[nodiscard]] int Slot() const { return *textureSlot; }

        [[nodiscard]] TextureTarget Target() const { return Handler().target; }
        [[nodiscard]] int TargetI() const { return (int)Target(); }
        void SetTarget(const TextureTarget target) { Handler().target = target; }

        [[nodiscard]] const Maths::uvec3& Size() const { return size; }
        [[nodiscard]] Maths::uvec2 Size2D() const { return size.xy(); }
        [[nodiscard]] uint Size1D() const { return size.x; }

        [[nodiscard]] OPENGL_API int Dimension() const;

        friend class FrameBuffer;
    };
}
