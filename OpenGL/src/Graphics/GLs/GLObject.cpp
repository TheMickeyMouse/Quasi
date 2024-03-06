#include <GL/glew.h>
#include "GLObject.h"
#include "GLDebug.h"

namespace Graphics {
    template <BufferType T> glID BufferHandler<T>::Create() const {
        glID id;
        GL_CALL(glGenBuffers(1, &id));
        Bind(id);
        return id;
    }

    template <BufferType T> void BufferHandler<T>::Destroy(const glID id) const {
        GL_CALL(glDeleteBuffers(1, &id));
    }

    template <BufferType T> void BufferHandler<T>::Bind(glID id) const {
        GL_CALL(glBindBuffer((int)T, id));
    }

    template <BufferType T> void BufferHandler<T>::Unbind() const {
        GL_CALL(glBindBuffer((int)T, 0));
    }

    template struct BufferHandler<BufferType::VERTEX>;
    template struct BufferHandler<BufferType::INDEX>;
}
