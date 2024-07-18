#include "VertexArray.h"

#include <GL/glew.h>
#include "GLDebug.h"

namespace Quasi::Graphics {
    VertexArray::VertexArray(GraphicsID id) : GLObject(id) {}

    VertexArray VertexArray::New() {
        GraphicsID id;
        Q_GL_CALL(glGenVertexArrays(1, &id));
        return VertexArray { id };
    }

    void VertexArray::DestroyObject(const GraphicsID id) {
        Q_GL_CALL(glDeleteVertexArrays(1, &id));
    }

    void VertexArray::BindObject(const GraphicsID id) {
        Q_GL_CALL(glBindVertexArray(id));
    }

    void VertexArray::UnbindObject() {
        Q_GL_CALL(glBindVertexArray(0));
    }

    void VertexArray::AddBuffer(const VertexBufferLayout& layout) {
        const Vec<VertexBufferComponent>& elements = layout.GetComponents();
        usize offset = 0;
        for (u32 i = 0; i < elements.size(); i++) {
            const auto& elem = elements[i];
            Q_GL_CALL(glEnableVertexAttribArray(i));
            if (elem.flags & VertexBufferComponent::INTEGER_FLAG)
                Q_GL_CALL(glVertexAttribIPointer(i, elem.count, elem.type->glID, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            else
                Q_GL_CALL(glVertexAttribPointer(i, elem.count, elem.type->glID, elem.flags & elem.NORMALIZED_FLAG, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            offset += elem.count * elem.type->typeSize;
        }
    }

    void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
        Bind();
        vb.Bind();
        AddBuffer(layout);
    }
}
