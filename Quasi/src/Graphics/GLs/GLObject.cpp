#include <GL/glew.h>
#include "GLObject.h"
#include "GLDebug.h"

namespace Quasi::Graphics {
    template <BufferType T> GraphicsID BufferHandler<T>::Create() const {
        GraphicsID id;
        GL_CALL(glGenBuffers(1, &id));
        Bind(id);
        return id;
    }

    template <BufferType T> void BufferHandler<T>::Destroy(const GraphicsID id) const {
        GL_CALL(glDeleteBuffers(1, &id));
    }

    template <BufferType T> void BufferHandler<T>::Bind(GraphicsID id) const {
        GL_CALL(glBindBuffer((int)T, id));
    }

    template <BufferType T> void BufferHandler<T>::Unbind() const {
        GL_CALL(glBindBuffer((int)T, 0));
    }

    template struct BufferHandler<BufferType::VERTEX>;
    template struct BufferHandler<BufferType::INDEX>;
}
