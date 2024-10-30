#include "VertexArray.h"

#include <glp.h>
#include "GLDebug.h"

namespace Quasi::Graphics {
    VertexArray::VertexArray(GraphicsID id) : GLObject(id) {}

    VertexArray VertexArray::New() {
        GraphicsID id;
        QGLCall$(GL::GenVertexArrays(1, &id));
        return VertexArray { id };
    }

    void VertexArray::DestroyObject(const GraphicsID id) {
        QGLCall$(GL::DeleteVertexArrays(1, &id));
    }

    void VertexArray::BindObject(const GraphicsID id) {
        QGLCall$(GL::BindVertexArray(id));
    }

    void VertexArray::UnbindObject() {
        QGLCall$(GL::BindVertexArray(0));
    }

    void VertexArray::AddBuffer(const VertexBufferLayout& layout) {
        const Vec<VertexBufferComponent>& elements = layout.GetComponents();
        usize offset = 0;
        for (u32 i = 0; i < elements.size(); i++) {
            const auto& elem = elements[i];
            QGLCall$(GL::EnableVertexAttribArray(i));
            if (elem.flags & VertexBufferComponent::INTEGER_FLAG)
                QGLCall$(GL::VertexAttribIPointer(i, elem.count, elem.type->glID, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            else
                QGLCall$(GL::VertexAttribPointer(i, elem.count, elem.type->glID, elem.flags & elem.NORMALIZED_FLAG, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            offset += elem.count * elem.type->typeSize;
        }
    }

    void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
        Bind();
        vb.Bind();
        AddBuffer(layout);
    }
}
