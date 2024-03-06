#include "Texture.h"
#include "GL/glew.h"
#include "GLDebug.h"
#include "GraphicsDevice.h"
#include "../vendor/stb_image/stb_image.h"

namespace Graphics {
    glID TextureHandler::Create() const {
        glID id;
        GL_CALL(glGenTextures(1, &id));
        return id;
    }

    void TextureHandler::Destroy(const glID id) const {
        GL_CALL(glDeleteTextures(1, &id));
    }

    void TextureHandler::Bind(glID id) const {
        GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
    }

    void TextureHandler::Unbind() const {
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    void TextureSlotHandler::operator()(int slot) const {
        if (slot == -1) return;
        Texture::Slots[slot] = nullptr;
    }

    void Texture::Init() {
        int textureCount = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureCount);
        GLLogger().Info({"Texture count: {}"}, textureCount);

        SlotCount = textureCount;
        Slots.resize(textureCount, nullptr);
    }

    Texture::Texture(const uchar* raw, uint w, uint h, bool useLinear,
                     TextureFormat format, TextureInternalFormat iformat, int alignment)
        : GLObject({}), size { w, h } {
        //flips texture
        LoadTexture(raw, useLinear, format, iformat, alignment);
    }

    Texture::Texture(const std::string& filePath, bool useLinear)
        : GLObject({}) {
        //flips texture
        stbi_set_flip_vertically_on_load(1);
        const uchar* localTexture = stbi_load(filePath.c_str(), (int*)&size.x, (int*)&size.y, &BPPixel, 4);
        LoadTexture(localTexture, useLinear);
        if (localTexture) stbi_image_free((void*)localTexture);
    }

    void Texture::LoadTexture(const uchar* img, bool useLinear,
                              TextureFormat format, TextureInternalFormat iformat, int alignment) {
        Bind();
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useLinear ? GL_LINEAR : GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, useLinear ? GL_LINEAR : GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, (int)iformat, size.x, size.y, 0, (int)format, (int)GLTypeID::UBYTE, img));
        Unbind();
    }

    Texture Texture::LoadPNGBytes(const uchar* png, int len, bool useLinear) {
        Maths::uvec2 size;
        int BPPixel;
        stbi_set_flip_vertically_on_load(1);
        const uchar* localTexture = stbi_load_from_memory(png, len, (int*)&size.x, (int*)&size.y, &BPPixel, 4);
        Texture tex { localTexture, size.x, size.y, useLinear };
        if (localTexture) stbi_image_free((void*)localTexture);
        
        return tex;
    }

    void Texture::Activate(int slot) {
        if (slot == -1 && textureSlot.is_null())
            slot = FindEmptySlot();
        if (slot == -1) return; // if slot is still -1, then the slots are full

        GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
        Bind();
        Slots[slot] = this;
        textureSlot.reset(slot);
    }

    void Texture::Deactivate() {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + *textureSlot));
        Unbind();
        textureSlot.reset(-1);
    }

    void Texture::DeactivateAll() {
        for (Texture* t : Slots) {
            if (t) t->Deactivate();
        }
    }

    int Texture::FindEmptySlot() {
        for (int i = 0; i < SlotCount; ++i) {
            if (Slots[i] == nullptr)
                return i;
        }
        return -1;
    }

    void Texture::SetSubTexture(const uchar* data, Maths::rect2u rect, TextureFormat format) {
        GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
            rect.min.x, rect.min.y, rect.width(), rect.height(),
            (int)format, (int)GLTypeID::UBYTE, data));
    }
}
