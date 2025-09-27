#include "Texture.h"

#include <glp.h>
#include "Utils/CStr.h"
#include "Utils/Vec.h"
#include "GLDebug.h"
#include "GraphicsDevice.h"
#include "Image.h"
#include "vendor/stb_image/stb_image.h"

namespace Quasi::Graphics {
    void STBIImageHandler::operator()(byte* dat) const {
        stbi_image_free(dat);
    }

    void TextureBase::DestroyObject(GraphicsID id) {
        QGLCall$(GL::DeleteTextures(1, &id));
    }

    void TextureBase::BindObject(TextureTarget target, GraphicsID id) {
        QGLCall$(GL::BindTexture((int)target, id));
    }

    void TextureBase::UnbindObject(TextureTarget target) {
        QGLCall$(GL::BindTexture((int)target, 0));
    }

    void TextureBase::SetSample(TextureTarget target, TextureSample sample) {
        SetParam(target, TextureParamName::MIN_FILTER, (int)sample);
        SetParam(target, TextureParamName::MAG_FILTER, (int)sample);
    }

    void TextureBase::SetWrapping(TextureTarget target, int dim, TextureBorder border) {
        switch (dim) {
            case 3:
                SetParam(target, TextureParamName::WRAP_R, (int)border);
                [[fallthrough]];
            case 2:
                SetParam(target, TextureParamName::WRAP_T, (int)border);
                [[fallthrough]];
            case 1:
                SetParam(target, TextureParamName::WRAP_S, (int)border);
                break;
            default:;
        }
    }

    void TextureBase::SetParam(TextureTarget target, TextureParamName paramName, int value) {
        QGLCall$(GL::TexParameteri((int)target, (int)paramName, value));
    }

    void TextureBase::SetParam(TextureTarget target, TextureParamName paramName, float value) {
        QGLCall$(GL::TexParameterf((int)target, (int)paramName, value));
    }

    void TextureBase::SetParam(TextureTarget target, TextureParamName paramName, const int* value) {
        QGLCall$(GL::TexParameteriv((int)target, (int)paramName, value));
    }

    void TextureBase::SetParam(TextureTarget target, TextureParamName paramName, const float* value) {
        QGLCall$(GL::TexParameterfv((int)target, (int)paramName, value));
    }

    void TextureBase::SetPixelStore(PixelStoreParam param, int val) {
        QGLCall$(GL::PixelStorei((int)param, val));
    }

    void TextureBase::Activate(TextureTarget target, int slot) const {
        QGLCall$(GL::ActiveTexture(GL::TEXTURE0 + slot));
        BindObject(target, rendererID);
    }

    TextureBase::TextureBase(GraphicsID id) : GLObject(id) {}

    template <TextureTarget Target>
    TextureObject<Target>::TextureObject(GraphicsID id, const Math::Vector<int, DIM>& size)
        : TextureBase(id), size(size) {}

    template <TextureTarget Target>
    void TextureObject<Target>::DefaultParams(bool pixelated, TextureBorder b) const {
        SetSample(pixelated ? TextureSample::NEAREST : TextureSample::LINEAR);
        SetWrapping(b);
    }

    template <TextureTarget Target>
    void TextureObject<Target>::LoadTexture(const byte* img, const TextureLoadParams& loadMode) {
        Bind();
        DefaultParams(loadMode.pixelated, loadMode.border);
        TexImage(img, size, loadMode);
        Unbind();
    }

    template <TextureTarget Target>
    TextureObject<Target> TextureObject<Target>::New(const byte* raw, const Math::Vector<int, DIM>& size, const TextureLoadParams& loadMode) {
        GraphicsID rendererID;
        QGLCall$(GL::GenTextures(1, &rendererID));
        TextureObject t { rendererID, size };
        t.LoadTexture(raw, loadMode);
        return t;
    }

    template <TextureTarget Target>
    TextureObject<Target> TextureObject<Target>::New(const Image& image, const TextureLoadParams& loadMode) requires (Target == _2D) {
        GraphicsID rendererID;
        QGLCall$(GL::GenTextures(1, &rendererID));
        TextureObject t { rendererID, image.Size() };
        t.LoadTexture(image.Data(), loadMode);
        return t;
    }

    template <TextureTarget Target>
    TextureObject<Target> TextureObject<Target>::LoadPNGBytes(Bytes pngbytes, const TextureLoadParams& loadMode) requires (Target == _2D) {
        const Image img = Image::LoadPNGBytes(pngbytes);
        return New(img.Data(), img.Size(), loadMode);
    }

    template <TextureTarget Target>
    TextureObject<Target> TextureObject<Target>::LoadPNG(CStr fname, const TextureLoadParams& loadMode) requires (Target == _2D) {
        const Image img = Image::LoadPNG(fname);
        return New(img.Data(), img.Size(), loadMode);
    }

    template <TextureTarget Target>
    TextureObject<Target> TextureObject<Target>::LoadCubemapPNG(IList<CStr> faces, const TextureLoadParams& loadMode) requires (Target == CUBEMAP) {
        if (faces.size() != 6) return {};

        stbi_set_flip_vertically_on_load(0);
        TextureObject cubemap {};
        cubemap.Bind();
        int faceTarget = (int)CUBEMAP_RIGHT;
        for (CStr face : faces) {
            int sx, sy, bpx;
            const auto localTexture = STBIImage::Own(stbi_load(face.Data(), &sx, &sy, &bpx, 4));
            QGLCall$(GL::TexImage2D(
                faceTarget, 0, (int)loadMode.internalformat, sx, sy, 0,
                (int)loadMode.format, loadMode.type->glID, localTexture));
            ++faceTarget;
        }
        cubemap.DefaultParams(loadMode.pixelated, loadMode.border);
        cubemap.Unbind();

        return cubemap;
    }

    template <TextureTarget Target>
    void TextureObject<Target>::Activate(int slot) {
        QGLCall$(GL::ActiveTexture(GL::TEXTURE0 + slot));
        Bind();
    }

    template <TextureTarget Target>
    void TextureObject<Target>::BindImageTexture(int slot, int mipmapLevel, Access access, TextureIFormat format) {
        QGLCall$(GL::BindImageTexture(slot, rendererID, mipmapLevel, 0, 0, (int)access, (int)format));
    }

    template <TextureTarget Target>
    void TextureObject<Target>::SetSubTexture(const void* data, const Math::Rect<int, DIM>& rect, const TextureLoadParams& params) {
        if constexpr (DIM == 1) {
            QGLCall$(GL::TexSubImage1D((int)Target, params.level, rect.min.x, rect.Width(), (int)params.format, params.type->glID, data));
        } else if constexpr (DIM == 2) {
            QGLCall$(GL::TexSubImage2D((int)Target, params.level, rect.min.x, rect.min.y, rect.Width(), rect.Height(), (int)params.format, params.type->glID, data));
        } else if constexpr (DIM == 3) {
            QGLCall$(GL::TexSubImage3D((int)Target, params.level, rect.min.x, rect.min.y, rect.min.z, rect.Width(), rect.Height(), rect.Depth(), (int)params.format, params.type->glID, data));
        }
    }

    template <TextureTarget Target>
    void TextureObject<Target>::TexImage(const byte* data, const Math::Vector<int, DIM>& dim, const TextureLoadParams& params) {
        if constexpr (DIM == 1) {
            for (u32 level = 0; level <= params.level; ++level) {
                QGLCall$(GL::TexImage1D((int)Target, level, (int)params.internalformat, dim.x >> level, 0, (int)params.format, params.type->glID, data));
            }
        } else if constexpr (DIM == 2) {
            for (u32 level = 0; level <= params.level; ++level) {
                QGLCall$(GL::TexImage2D((int)Target, level, (int)params.internalformat, dim.x >> level, dim.y >> level, 0, (int)params.format, params.type->glID, data));
            }
        } else if constexpr (DIM == 3) {
            for (u32 level = 0; level <= params.level; ++level) {
                QGLCall$(GL::TexImage3D((int)Target, level, (int)params.internalformat, dim.x >> level, dim.y >> level, dim.z >> level, 0, (int)params.format, params.type->glID, data));
            }
        }
    }

    template class TextureObject<TextureTarget::_1D>;
    template class TextureObject<TextureTarget::_2D>;
    template class TextureObject<TextureTarget::_3D>;
    template class TextureObject<TextureTarget::ARRAY_1D>;
    template class TextureObject<TextureTarget::ARRAY_2D>;
    template class TextureObject<TextureTarget::CUBEMAP>;
}
