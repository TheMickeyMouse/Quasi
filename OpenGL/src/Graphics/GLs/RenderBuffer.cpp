#include "RenderBuffer.h"
#include "GL/glew.h"
#include "GLDebug.h"

namespace Graphics {
    glID RenderBufferHandler::Create() const {
        glID id;
        GL_CALL(glGenRenderbuffers(1, &id));
        return id;
    }

    void RenderBufferHandler::Destroy(glID id) const {
        GL_CALL(glDeleteRenderbuffers(1, &id));
    }

    void RenderBufferHandler::Bind(glID id) const {
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    }

    void RenderBufferHandler::Unbind() const {
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }

    RenderBuffer::RenderBuffer(TextureIFormat format, Maths::ivec2 size) : GLObject({}) {
        Bind();
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, (int)format, size.x, size.y));
        Unbind();
    }
}
