#include "RenderBuffer.h"
#include "GL/glew.h"
#include "GLDebug.h"

namespace Quasi::Graphics {
    GraphicsID RenderBufferHandler::Create() const {
        GraphicsID id;
        GL_CALL(glGenRenderbuffers(1, &id));
        return id;
    }

    void RenderBufferHandler::Destroy(GraphicsID id) const {
        GL_CALL(glDeleteRenderbuffers(1, &id));
    }

    void RenderBufferHandler::Bind(GraphicsID id) const {
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    }

    void RenderBufferHandler::Unbind() const {
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }

    RenderBuffer::RenderBuffer(TextureIFormat format, Math::iVector2 size) {
        Create();
        Bind();
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, (int)format, size.x, size.y));
        Unbind();
    }
}
