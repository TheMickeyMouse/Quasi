#include "Texture.h"

#include <ranges>

#include "GL/glew.h"
#include "GLDebug.h"
#include "GraphicsDevice.h"
#include "stb_image/stb_image.h"

namespace Quasi::Graphics {
    GraphicsID TextureHandler::Create() const {
        GraphicsID id;
        GL_CALL(glGenTextures(1, &id));
        return id;
    }

    void TextureHandler::Destroy(const GraphicsID id) const {
        GL_CALL(glDeleteTextures(1, &id));
    }

    void TextureHandler::Bind(GraphicsID id) const {
        GL_CALL(glBindTexture((int)target, id));
    }

    void TextureHandler::Unbind() const {
        GL_CALL(glBindTexture((int)target, 0));
    }

    void TextureSlotHandler::operator()(void* slot) const {
        if (!slot) return;
        Texture::Slots[(usize)slot - 1] = nullptr;
    }

    void STBIImageHandler::operator()(void* dat) const {
        stbi_image_free(dat);
    }

    void Texture::Init() {
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &SlotCount);
        GLLogger().Info("Texture count: {}", SlotCount);

        Slots.resize(SlotCount, nullptr);
    }

    Texture::Texture(const byte* raw, const Math::uVector3& size, const TextureInitParams& init)
        : size(size) {
        Create();
        // flips texture
        SetTarget(init.target);
        LoadTexture(raw, init);
    }

    void Texture::SetParam(TextureParamName param, float val) const {
        GL_CALL(glTexParameterf(TargetI(), (int)param, val));
    }

    void Texture::SetParam(TextureParamName param, int val) const {
        GL_CALL(glTexParameteri(TargetI(), (int)param, val));
    }

    void Texture::SetParam(TextureParamName param, const float* vals) const {
        GL_CALL(glTexParameterfv(TargetI(), (int)param, vals));
    }

    void Texture::SetParam(TextureParamName param, const int* vals) const {
        GL_CALL(glTexParameterIiv(TargetI(), (int)param, vals));
    }

    void Texture::DefaultParams() const {
        SetSample(TextureSample::LINEAR);
        SetWrapping(TextureBorder::CLAMP_TO_EDGE);
    }

    void Texture::TexImage(const byte* data, const Math::uVector3& dim, const TextureLoadParams& params, TextureTarget overrideTarget) {
        const int newTarget = overrideTarget == TextureTarget::NONE ? TargetI() : (int)overrideTarget;
        switch (Dimension()) {
            case 1:
                return GL_CALL(glTexImage1D(newTarget, params.level, (int)params.internalformat, dim.x, 0, (int)params.format, (int)params.type, data));
            case 2:
                return GL_CALL(glTexImage2D(newTarget, params.level, (int)params.internalformat, dim.x, dim.y, 0, (int)params.format, (int)params.type, data));
            case 3:
                return GL_CALL(glTexImage3D(newTarget, params.level, (int)params.internalformat, dim.x, dim.y, dim.z, 0, (int)params.format, (int)params.type, data));
            default:;
        }
    }

    void Texture::SetPixelStore(PixelStoreParam param, int val) {
        GL_CALL(glPixelStorei((int)param, val));
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
        const STBIImage localTexture { stbi_load_from_memory(datapng.data(), (int)datapng.size(), &size.x, &size.y, &BPPixel, 4) };
        Texture tex { (const byte*)localTexture.get(), { size.x, size.y }, init };
        
        return tex;
    }

    Texture Texture::LoadPNG(Str fname, const TextureInitParams& init) {
        Math::iVector2 size;
        int BPPixel;
        stbi_set_flip_vertically_on_load(1);
        Debug::Assert(!*fname.end(), "filename doesn't have null terminator");
        const STBIImage localTexture { stbi_load(fname.data(), &size.x, &size.y, &BPPixel, 4) };
        Texture tex { (const byte*)localTexture.get(), { size.x, size.y }, init };

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
            Debug::Assert(!*face.end(), "filename of cubemap doesn't have null terminator");
            int sx, sy, bpx;
            const STBIImage localTexture { stbi_load(face.data(), &sx, &sy, &bpx, 4) };
            Texture dummy {};
            dummy.SetTarget((TextureTarget)faceTarget);
            dummy.TexImage((const byte*)localTexture.get(), { sx, sy }, init.load);
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

        GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
        Bind();
        Slots[slot] = *this;
        textureSlot.reset((void*)(usize)(slot + 1));
    }

    void Texture::Deactivate() {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + Slot()));
        Unbind();
        textureSlot.reset(nullptr);
    }

    void Texture::DeactivateAll() {
        for (Ref<Texture> t : Slots) {
            if (t) t->Deactivate();
        }
    }

    int Texture::FindEmptySlot() {
        for (u32 i = 0; i < SlotCount; ++i) {
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
                return GL_CALL(glTexSubImage1D(TargetI(), params.level, rect.min.x, rect.width(), (int)params.format, (int)params.type, data));
            case 2:
                return GL_CALL(glTexSubImage2D(TargetI(), params.level, rect.min.x, rect.min.y, rect.width(), rect.height(), (int)params.format, (int)params.type, data));
            case 3:
                return GL_CALL(glTexSubImage3D(TargetI(), params.level, rect.min.x, rect.min.y, rect.min.z, rect.width(), rect.height(), rect.depth(), (int)params.format, (int)params.type, data));
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
