#include "RenderBuffer.h"
#include <glp.h>
#include "GLDebug.h"

namespace Quasi::Graphics {
    RenderBuffer::RenderBuffer(GraphicsID id) : GLObject(id) {}

    RenderBuffer RenderBuffer::New(TextureIFormat format, Math::iVector2 size) {
        GraphicsID id;
        Q_GL_CALL(GL::GenRenderbuffers(1, &id));
        BindObject(id);
        Q_GL_CALL(GL::RenderbufferStorage(GL::RENDERBUFFER, (int)format, size.x, size.y));
        UnbindObject();
        return RenderBuffer { id };
    }

    void RenderBuffer::DestroyObject(GraphicsID id) {
        Q_GL_CALL(GL::DeleteRenderbuffers(1, &id));
    }

    void RenderBuffer::BindObject(GraphicsID id) {
        Q_GL_CALL(GL::BindRenderbuffer(GL::RENDERBUFFER, id));
    }

    void RenderBuffer::UnbindObject() {
        Q_GL_CALL(GL::BindRenderbuffer(GL::RENDERBUFFER, 0));
    }
}
