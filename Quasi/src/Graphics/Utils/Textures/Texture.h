#pragma once
#include <vector>

#include "Utils/Box.h"
#include "Graphics/GLs/GLObject.h"

#include "Graphics/GLs/GLTypeID.h"
#include "TextureConstants.h"
#include "Math/Rect.h"
#include "Utils/Variant.h"

namespace Quasi {
    struct CStr;
}

namespace Quasi::Graphics {
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

    struct STBIImageHandler { void operator()(byte* dat) const; };
    using STBIImage = Box<byte, STBIImageHandler>;

    class Texture : public GLObject<Texture> {
    public:
        inline static int SlotCount = -1;
        inline static Vec<OptRef<Texture>> Slots {};
        static void Init();
    private:
        struct TextureSlot : IResource<u32, TextureSlot> {
        private:
            u32 index;
            TextureSlot(u32 slot) : index(slot) {}
        public:
            TextureSlot() : index(0) {}
            TextureSlot(Nullptr) : index(0) {}
            TextureSlot(TextureSlot&& ts) noexcept { MoveConstructOp(ts); }
            TextureSlot& operator=(TextureSlot&& ts) noexcept { MoveAssignOp(ts); return *this; }

            static TextureSlot AtSlot(u32 slot) { return { slot }; }
            static TextureSlot Unset() { return {}; }

            void CloseImpl();
            const u32& GetImpl() const { return index; }
            u32& GetMutImpl() { return index; }
            void TransferImpl(TextureSlot& dst) { dst.index = index; index = 0; }

            void Replace(u32 slot) { CloseImpl(); index = slot; }

            operator bool() const { return index != 0; }
        };

        Math::uVector3 size;
        TextureSlot textureSlot;
        TextureTarget target = TextureTarget::NONE;

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
        static Texture LoadPNG(CStr fname, const TextureInitParams& init = {});

        static Texture LoadCubemapPNG(IList<CStr> faces /* in order: rludfb */, const TextureInitParams& init = {});

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
        void TexImage(const byte* data, u32 width, const TextureLoadParams& params = {}) {
            TexImage(data, { width, 0, 0 }, params);
        }
        void TexImage(const byte* data, const Math::uVector2& dim, const TextureLoadParams& params = {}) {
            TexImage(data, dim.with_z(0), params);
        }

        static void SetPixelStore(PixelStoreParam param, int val);

        int Slot() const { return (int)(*textureSlot - 1); }

        TextureTarget Target() const { return target; }
        int TargetI() const { return (int)Target(); }
        void SetTarget(const TextureTarget t) { target = t; }

        const Math::uVector3& Size() const { return size; }
        Math::uVector2 Size2D() const { return size.xy(); }
        uint Size1D() const { return size.x; }

        int Dimension() const;

        friend class FrameBuffer;
    };
}
