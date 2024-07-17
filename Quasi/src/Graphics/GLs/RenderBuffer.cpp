#include "RenderBuffer.h"
#include "GL/glew.h"
#include "GLDebug.h"

namespace Quasi::Graphics {
    RenderBuffer::RenderBuffer(GraphicsID id) : GLObject(id) {}

    RenderBuffer RenderBuffer::New(TextureIFormat format, Math::iVector2 size) {
        GraphicsID id;
        Q_GL_CALL(glGenRenderbuffers(1, &id));
        BindObject(id);
        Q_GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, (int)format, size.x, size.y));
        UnbindObject();
        return RenderBuffer { id };
    }

    void RenderBuffer::DestroyObject(GraphicsID id) {
        Q_GL_CALL(glDeleteRenderbuffers(1, &id));
    }

    void RenderBuffer::BindObject(GraphicsID id) {
        Q_GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
    }

    void RenderBuffer::UnbindObject() {
        Q_GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    }
}
