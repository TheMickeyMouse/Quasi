#include <GL/glew.h>
#include "FrameBuffer.h"

#include "Debugging.h"
#include "RenderBuffer.h"

namespace Graphics {
    glID FrameBufferHandler::Create() const {
        glID id;
        GLCALL(glGenFramebuffers(1, &id));
        return id;
    }

    void FrameBufferHandler::Destroy(glID id) const {
        GLCALL(glDeleteFramebuffers(1, &id));
    }

    void FrameBufferHandler::Bind(glID id) const {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
    }

    void FrameBufferHandler::Unbind() const {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void FrameBuffer::Attach(const Texture& tex) const {
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.rendererID, 0));
    }

    void FrameBuffer::Attach(const RenderBuffer& rbo, AttachmentType type) const {
        GLCALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, (int)type, GL_RENDERBUFFER, rbo.rendererID));
    }

    void FrameBuffer::Complete() const {
        const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LOG("[ERR]: err when completing framebuffer with code " << status);
        }
    }
}
