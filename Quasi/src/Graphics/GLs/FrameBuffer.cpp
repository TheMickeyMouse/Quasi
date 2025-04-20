#include "FrameBuffer.h"

#include <glp.h>

#include "RenderBuffer.h"
#include "GLDebug.h"
#include "Texture.h"

namespace Quasi::Graphics {
    FrameBuffer::FrameBuffer(GraphicsID id) : GLObject(id) {}

    FrameBuffer FrameBuffer::New() {
        GraphicsID id;
        QGLCall$(GL::GenFramebuffers(1, &id));
        return FrameBuffer { id };
    }

    void FrameBuffer::DestroyObject(GraphicsID id) {
        QGLCall$(GL::DeleteFramebuffers(1, &id));
    }

    void FrameBuffer::BindObject(GraphicsID id) {
        QGLCall$(GL::BindFramebuffer(GL::FRAMEBUFFER, id));
    }

    void FrameBuffer::UnbindObject() {
        QGLCall$(GL::BindFramebuffer(GL::FRAMEBUFFER, 0));
    }

    void FrameBuffer::Attach(const Texture& tex, AttachmentType type) const {
        QGLCall$(GL::FramebufferTexture2D(GL::FRAMEBUFFER, (int)type, tex.TargetI(), tex.rendererID, 0));
    }

    void FrameBuffer::Attach(const RenderBuffer& rbo, AttachmentType type) const {
        QGLCall$(GL::FramebufferRenderbuffer(GL::FRAMEBUFFER, (int)type, GL::RENDERBUFFER, rbo.rendererID));
    }

    void FrameBuffer::Complete() const {
        const int status = GL::CheckFramebufferStatus(GL::FRAMEBUFFER);
        if (status != GL::FRAMEBUFFER_COMPLETE) {
            GLLogger().QError$("Framebuffer was incomplete with code 0x{:04X}.", status);
        }
    }
}
