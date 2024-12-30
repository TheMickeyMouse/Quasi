#include "Texture.h"

#include <ranges>

#include <glp.h>
#include "GLDebug.h"
#include "GraphicsDevice.h"
#include "stb_image/stb_image.h"

namespace Quasi::Graphics {
    void STBIImageHandler::operator()(byte* dat) const {
        stbi_image_free(dat);
    }

    void Texture::TextureSlot::CloseImpl() {
        if (!index) return;
        Texture::Slots[(usize)index - 1] = nullptr;
    }

    void Texture::Init() {
        GL::GetIntegerv(GL::MAX_TEXTURE_IMAGE_UNITS, &SlotCount);
        GLLogger().QInfo$("Texture count: {}", SlotCount);

        Slots.Resize(SlotCount, nullptr);
    }

    Texture::Texture(GraphicsID id, const Math::uVector3& size) : GLObject(id), size(size), textureSlot(TextureSlot::Unset()) {}

    Texture Texture::New(const byte* raw, const Math::uVector3& size, const TextureInitParams& init) {
        GraphicsID rendererID;
        QGLCall$(GL::GenTextures(1, &rendererID));
        Texture t { rendererID, size };
        t.SetTarget(init.target);
        t.LoadTexture(raw, init);
        return t;
    }

    void Texture::DestroyObject(const GraphicsID id) {
        QGLCall$(GL::DeleteTextures(1, &id));
    }

    void Texture::BindObject(TextureTarget target, GraphicsID id) {
        QGLCall$(GL::BindTexture((int)target, id));
    }

    void Texture::UnbindObject(TextureTarget target) {
        QGLCall$(GL::BindTexture((int)target, 0));
    }

    void Texture::SetParam(TextureParamName param, float val) const {
        QGLCall$(GL::TexParameterf(TargetI(), (int)param, val));
    }

    void Texture::SetParam(TextureParamName param, int val) const {
        QGLCall$(GL::TexParameteri(TargetI(), (int)param, val));
    }

    void Texture::SetParam(TextureParamName param, const float* vals) const {
        QGLCall$(GL::TexParameterfv(TargetI(), (int)param, vals));
    }

    void Texture::SetParam(TextureParamName param, const int* vals) const {
        QGLCall$(GL::TexParameterIiv(TargetI(), (int)param, vals));
    }

    void Texture::DefaultParams() const {
        SetSample(TextureSample::LINEAR);
        SetWrapping(TextureBorder::CLAMP_TO_EDGE);
    }

    void Texture::TexImage(const byte* data, const Math::uVector3& dim, const TextureLoadParams& params, TextureTarget overrideTarget) {
        const int newTarget = overrideTarget == TextureTarget::NONE ? TargetI() : (int)overrideTarget;
        switch (Dimension()) {
            case 1:
                return QGLCall$(GL::TexImage1D(newTarget, params.level, (int)params.internalformat, dim.x, 0, (int)params.format, params.type->glID, data));
            case 2:
                return QGLCall$(GL::TexImage2D(newTarget, params.level, (int)params.internalformat, dim.x, dim.y, 0, (int)params.format, params.type->glID, data));
            case 3:
                return QGLCall$(GL::TexImage3D(newTarget, params.level, (int)params.internalformat, dim.x, dim.y, dim.z, 0, (int)params.format, params.type->glID, data));
            default:;
        }
    }

    void Texture::SetPixelStore(PixelStoreParam param, int val) {
        QGLCall$(GL::PixelStorei((int)param, val));
    }

    void Texture::LoadTexture(const byte* img, const TextureInitParams& init) {
        Bind();
        DefaultParams();
        SetParams(init.params);
        TexImage(img, size, init.load);
        Unbind();
    }


    Texture Texture::LoadPNGBytes(Span<const byte> datapng, const TextureInitParams& init) {
        Math::iVector2 size;
        int BPPixel;
        stbi_set_flip_vertically_on_load(1);
        const auto localTexture = STBIImage::Own(stbi_load_from_memory(datapng.Data(), (int)datapng.Length(), &size.x, &size.y, &BPPixel, 4));
        Texture tex = New(localTexture, { size.x, size.y }, init);
        
        return tex;
    }

    Texture Texture::LoadPNG(Str fname, const TextureInitParams& init) {
        Math::iVector2 size;
        int BPPixel;
        stbi_set_flip_vertically_on_load(1);
        Debug::AssertMsg(!*fname.end(), "filename doesn't have null terminator");
        const auto localTexture = STBIImage::Own(stbi_load(fname.data(), &size.x, &size.y, &BPPixel, 4));
        Texture tex = New(localTexture, { size.x, size.y }, init);

        return tex;
    }

    Texture Texture::LoadCubemapPNG(IList<Str> faces, const TextureInitParams& init) {
        if (faces.size() != 6) return {};

        stbi_set_flip_vertically_on_load(0);
        Texture cubemap {};
        cubemap.SetTarget(TextureTarget::CUBEMAP);
        cubemap.Bind();
        int faceTarget = (int)TextureTarget::CUBEMAP_RIGHT;
        for (Str face : faces) {
            Debug::AssertMsg(!*face.end(), "filename of cubemap doesn't have null terminator");
            int sx, sy, bpx;
            const auto localTexture = STBIImage::Own(stbi_load(face.data(), &sx, &sy, &bpx, 4));
            Texture dummy {};
            dummy.SetTarget((TextureTarget)faceTarget);
            dummy.TexImage(localTexture, { sx, sy }, init.load);
            ++faceTarget;
        }
        cubemap.DefaultParams();
        cubemap.SetParams(init.params);
        cubemap.Unbind();

        return cubemap;
    }

    void Texture::Activate(int slot) {
        if (slot == -1 && !textureSlot)
            slot = FindEmptySlot();
        if (slot == -1) return; // if slot is still -1, then the slots are full

        QGLCall$(GL::ActiveTexture(GL::TEXTURE0 + slot));
        Bind();
        Slots[slot] = *this;
        textureSlot.Replace(slot + 1);
    }

    void Texture::Deactivate() {
        QGLCall$(GL::ActiveTexture(GL::TEXTURE0 + Slot()));
        Unbind();
        textureSlot.Close();
    }

    void Texture::DeactivateAll() {
        for (OptRef<Texture> t : Slots) {
            if (t) t->Deactivate();
        }
    }

    int Texture::FindEmptySlot() {
        for (i32 i = 0; i < SlotCount; ++i) {
            if (!Slots[i]) return i;
        }
        return -1;
    }

    void Texture::SetParams(TextureParameters params) {
        for (const TextureParamPair& p : params) {
            switch (p.pname) {
                case TextureParamName::XT_SAMPLE_FILTER:
                    SetSample  ((TextureSample)*p.val.As<int>());
                    break;
                case TextureParamName::XT_WRAPPING:
                    SetWrapping((TextureBorder)*p.val.As<int>());
                    break;
                default:
                    if      (p.val.Is<int>())          SetParam(p.pname, *p.val.As<int>());
                    else if (p.val.Is<float>())        SetParam(p.pname, *p.val.As<float>());
                    else if (p.val.Is<const int*>())   SetParam(p.pname, *p.val.As<const int*>());
                    else if (p.val.Is<const float*>()) SetParam(p.pname, *p.val.As<const float*>());
            }
        }
    }

    void Texture::SetSample(TextureSample s) const {
        SetParam(TextureParamName::MIN_FILTER, s);
        SetParam(TextureParamName::MAG_FILTER, s);
    }

    void Texture::SetWrapping(TextureBorder b) const {
        switch (Dimension()) {
            case 3:
                SetParam(TextureParamName::WRAP_R, b);
            case 2:
                SetParam(TextureParamName::WRAP_T, b);
            case 1:
                SetParam(TextureParamName::WRAP_S, b);
            default:;
        }
    }

    void Texture::SetSubTexture(const void* data, const Math::uRect3D& rect, const TextureLoadParams& params) {
        switch (Dimension()) {
            case 1:
                return QGLCall$(GL::TexSubImage1D(TargetI(), params.level, rect.min.x, rect.width(), (int)params.format, params.type->glID, data));
            case 2:
                return QGLCall$(GL::TexSubImage2D(TargetI(), params.level, rect.min.x, rect.min.y, rect.width(), rect.height(), (int)params.format, params.type->glID, data));
            case 3:
                return QGLCall$(GL::TexSubImage3D(TargetI(), params.level, rect.min.x, rect.min.y, rect.min.z, rect.width(), rect.height(), rect.depth(), (int)params.format, params.type->glID, data));
            default:;
        }
    }

    int Texture::Dimension() const {
        switch (Target()) {
            case TextureTarget::TEXTURE_1D:
            case TextureTarget::TEXTURE_1D_ARRAY:
                return 1;
            case TextureTarget::TEXTURE_2D:
            case TextureTarget::TEXTURE_2D_ARRAY:
            case TextureTarget::TEXTURE_2D_MULTISAMPLE:
            case TextureTarget::TEXTURE_2D_MULTISAMPLE_ARRAY:
            case TextureTarget::TEXTURE_RECTANGLE:
                return 2;
            case TextureTarget::TEXTURE_3D:
            case TextureTarget::CUBEMAP:
                return 3;
            case TextureTarget::CUBEMAP_POS_X:
            case TextureTarget::CUBEMAP_NEG_X:
            case TextureTarget::CUBEMAP_POS_Y:
            case TextureTarget::CUBEMAP_NEG_Y:
            case TextureTarget::CUBEMAP_POS_Z:
            case TextureTarget::CUBEMAP_NEG_Z:
                return 2;
            default: return 0;
        }
    }
}
