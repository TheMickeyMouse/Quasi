#include "RenderBuffer.h"

namespace Graphics {
    glID RenderBufferHandler::Create() const {
        glID id;
        GLCALL(glGenRenderbuffers(1, &id));
        return id;
    }

    void RenderBufferHandler::Destroy(glID id) const {
        GLCALL(glDeleteRenderbuffers(1, &id));
    }

    void RenderBufferHandler::Bind(glID id) const {
        GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    }

    void RenderBufferHandler::Unbind() const {
        GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }

    RenderBuffer::RenderBuffer(TextureInternalFormat format, Maths::ivec2 size) : GLObject({}) {
        Bind();
        GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, (int)format, size.x, size.y));
        Unbind();
    }
}
