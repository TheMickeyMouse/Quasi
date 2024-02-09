#include <GL/glew.h>
#include "GLObject.h"
#include "Debugging.h"

namespace Graphics {
    template <BufferType T> glID BufferHandler<T>::Create() const {
        glID id;
        GLCALL(glGenBuffers(1, &id));
        Bind(id);
        return id;
    }

    template <BufferType T> void BufferHandler<T>::Destroy(const glID id) const {
        GLCALL(glDeleteBuffers(1, &id));
    }

    template <BufferType T> void BufferHandler<T>::Bind(glID id) const {
        GLCALL(glBindBuffer((int)T, id));
    }

    template <BufferType T> void BufferHandler<T>::Unbind() const {
        GLCALL(glBindBuffer((int)T, 0));
    }

    template struct BufferHandler<BufferType::VERTEX>;
    template struct BufferHandler<BufferType::INDEX>;
}
