#include "FrameBuffer.h"

#include "RenderBuffer.h"

#include <GL/glew.h>
#include "GLDebug.h"
#include "Textures/Texture.h"

namespace Quasi::Graphics {
    GraphicsID FrameBufferHandler::Create() const {
        GraphicsID id;
        Q_GL_CALL(glGenFramebuffers(1, &id));
        return id;
    }

    void FrameBufferHandler::Destroy(GraphicsID id) const {
        Q_GL_CALL(glDeleteFramebuffers(1, &id));
    }

    void FrameBufferHandler::Bind(GraphicsID id) const {
        Q_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    }

    void FrameBufferHandler::Unbind() const {
        Q_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void FrameBuffer::Attach(const Texture& tex, AttachmentType type) const {
        Q_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, (int)type, tex.TargetI(), tex.rendererID, 0));
    }

    void FrameBuffer::Attach(const RenderBuffer& rbo, AttachmentType type) const {
        Q_GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, (int)type, GL_RENDERBUFFER, rbo.rendererID));
    }

    void FrameBuffer::Complete() const {
        const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            GLLogger().Error("Framebuffer was incomplete with code 0x{:04X}.", status);
        }
    }
}
