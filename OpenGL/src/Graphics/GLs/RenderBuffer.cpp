#include "RenderBuffer.h"

namespace Graphics {
    RenderBuffer::RenderBuffer(TextureInternalFormat format, Maths::ivec2 size) {
        GLCALL(glGenRenderbuffers(1, &rendererID));
        Bind();
        GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, (int)format, size.x, size.y));
        Unbind();
    }

    RenderBuffer::~RenderBuffer() {
        GLCALL(glDeleteRenderbuffers(1, &rendererID));
        rendererID = GL_NULL;
    }

    void RenderBuffer::Transfer(RenderBuffer& dest, RenderBuffer&& from) {
        dest.rendererID = from.rendererID;
        from.rendererID = GL_NULL;
    }

    void RenderBuffer::Bind() const {
        GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, rendererID));
    }

    void RenderBuffer::Unbind() const {
        GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }
}
