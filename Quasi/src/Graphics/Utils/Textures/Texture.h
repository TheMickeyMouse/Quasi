#pragma once
#include <vector>

#include "GLObject.h"

#include "GLTypeID.h"
#include "TextureConstants.h"
#include "Math/Rect.h"
#include "Utils/Variant.h"

namespace Quasi::Graphics {
    struct TextureSlotHandler {
        void operator()(void* slot) const;
    };

    struct TextureParamPair {
        TextureParamName pname;
        Variant<int, float, const int*, const float*> val;

        template <class E> requires std::is_enum_v<E>
        TextureParamPair(TextureParamName name, E val) : pname(name), val((int)val) {}
        TextureParamPair(TextureParamName name, const decltype(val)& val) : pname(name), val(val) {}

        static TextureParamPair NearestSample()   { return { TextureParamName::XT_SAMPLE_FILTER, TextureSample::NEAREST }; }
        static TextureParamPair LinearSample()    { return { TextureParamName::XT_SAMPLE_FILTER, TextureSample::LINEAR }; }
        static TextureParamPair ClampedBorder()   { return { TextureParamName::XT_WRAPPING, TextureBorder::CLAMP_TO_EDGE }; }
        static TextureParamPair ColoredBorder()   { return { TextureParamName::XT_WRAPPING, TextureBorder::CLAMP_TO_BORDER }; }
        static TextureParamPair MirroredBorder()  { return { TextureParamName::XT_WRAPPING, TextureBorder::MIRRORED_REPEAT }; }
        static TextureParamPair RepeatingBorder() { return { TextureParamName::XT_WRAPPING, TextureBorder::REPEAT }; }
    };

    using TextureParameters = IList<TextureParamPair>;

    struct TextureLoadParams {
        TextureFormat format = TextureFormat::RGBA;
        TextureIFormat internalformat = TextureIFormat::RGBA_8;
        GLTypeID type = GLTypeID::BYTE;
        int level = 0;
    };

    struct TextureInitParams {
        TextureLoadParams load = {};
        TextureTarget target = TextureTarget::TEXTURE_2D;
        TextureParameters params;
    };

    struct STBIImageHandler {
        void operator()(void* dat) const;
    };
    using STBIImage = UniqueRef<void, STBIImageHandler>;

    class Texture : public GLObject<Texture> {
    public:
        inline static int SlotCount = -1;
        inline static Vec<Ref<Texture>> Slots {};
        static void Init();
    private:
        using TextureSlot = UniqueRef<void, TextureSlotHandler>;

        Math::uVector3 size;
        TextureSlot textureSlot;
        TextureTarget target;

        void DefaultParams() const;

        void LoadTexture(const byte* img, const TextureInitParams& init = {});

        explicit Texture(GraphicsID id, const Math::uVector3& size);
    public:
        Texture() = default;
        static Texture New(const byte* raw, const Math::uVector3& size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_3D });
        static Texture New(const byte* raw, const Math::uVector2& size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_2D }) { return New(raw, size.with_z(0), init); }
        static Texture New(const byte* raw, uint size, const TextureInitParams& init = { .target = TextureTarget::TEXTURE_1D }) { return New(raw, { size, 0, 0 }, init); }
        static void DestroyObject(GraphicsID id);
        static void BindObject(TextureTarget target, GraphicsID id);
        static void UnbindObject(TextureTarget target);
        void Bind() const { BindObject(target, rendererID); }
        void Unbind() const { UnbindObject(target); }

        static Texture LoadPNGBytes(Span<const byte> datapng, const TextureInitParams& init = {});
        static Texture LoadPNG(Str fname, const TextureInitParams& init = {});

        static Texture LoadCubemapPNG(IList<Str> faces /* in order: rludfb */, const TextureInitParams& init = {});

        void Activate(int slot = -1);
        void Deactivate();
        static void DeactivateAll();
        static int FindEmptySlot();

        void SetParam(TextureParamName param, float val) const;
        void SetParam(TextureParamName param, int val) const;
        void SetParam(TextureParamName param, const float* vals) const;
        void SetParam(TextureParamName param, const int* vals) const;
        template <class E> requires std::is_enum_v<E>
        void SetParam(TextureParamName param, E val) const { SetParam(param, (int)val); }

        void SetParams(TextureParameters params);

        void SetSample(TextureSample s) const;
        void SetWrapping(TextureBorder b) const;

        void SetSubTexture(const void* data, const Math::uRect3D& rect, const TextureLoadParams& params = {});
        void SetSubTexture(const void* data, const Math::uRect2D& rect, const TextureLoadParams& params = {}) {
            SetSubTexture(data, Math::uRect3D { rect.min.with_z(0), rect.max.with_z(0) }, params);
        }
        void SetSubTexture(const uchar* data, const Math::uRange& rect, const TextureLoadParams& params = {}) {
            SetSubTexture(data, Math::uRect3D { { rect.min, 0, 0 }, { rect.max, 0, 0 } }, params);
        }

        void TexImage(const byte* data, const Math::uVector3& dim, const TextureLoadParams& params = {}, TextureTarget overrideTarget = TextureTarget::NONE);
        void TexImage(const byte* data, uint width, const TextureLoadParams& params = {}) {
            TexImage(data, { width, 0, 0 }, params);
        }
        void TexImage(const byte* data, const Math::uVector2& dim, const TextureLoadParams& params = {}) {
            TexImage(data, dim.with_z(0), params);
        }

        static void SetPixelStore(PixelStoreParam param, int val);

        [[nodiscard]] int Slot() const { return (int)((usize)textureSlot.get() - 1); }

        [[nodiscard]] TextureTarget Target() const { return target; }
        [[nodiscard]] int TargetI() const { return (int)Target(); }
        void SetTarget(const TextureTarget t) { target = t; }

        [[nodiscard]] const Math::uVector3& Size() const { return size; }
        [[nodiscard]] Math::uVector2 Size2D() const { return size.xy(); }
        [[nodiscard]] uint Size1D() const { return size.x; }

        [[nodiscard]] int Dimension() const;

        friend class FrameBuffer;
    };
}
