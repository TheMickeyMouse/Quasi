#include <GL/glew.h>
#include "FrameBuffer.h"

#include "Debugging.h"
#include "RenderBuffer.h"

namespace Graphics {
    FrameBuffer::FrameBuffer(int) {
        GLCALL(glGenFramebuffers(1, &rendererID));
    }

    FrameBuffer::~FrameBuffer() {
        GLCALL(glDeleteFramebuffers(1, &rendererID));
        rendererID = GL_NULL;
    }

    void FrameBuffer::Transfer(FrameBuffer& dest, FrameBuffer&& from) {
        dest.rendererID = from.rendererID;
        from.rendererID = GL_NULL;
    }

    void FrameBuffer::Bind() const {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, rendererID));
    }

    void FrameBuffer::Unbind() const {
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
