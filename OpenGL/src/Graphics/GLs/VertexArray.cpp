#include "VertexArray.h"

#include <GL/glew.h>
#include "GLDebug.h"

namespace Quasi::Graphics {
    GraphicsID VertexArrayHandler::Create() const {
        GraphicsID id;
        GL_CALL(glGenVertexArrays(1, &id));
        return id;
    }

    void VertexArrayHandler::Destroy(const GraphicsID id) const {
        GL_CALL(glDeleteVertexArrays(1, &id));
    }

    void VertexArrayHandler::Bind(const GraphicsID id) const {
        GL_CALL(glBindVertexArray(id));
    }

    void VertexArrayHandler::Unbind() const {
        GL_CALL(glBindVertexArray(0));
    }

    void VertexArray::AddBuffer(const VertexBufferLayout& layout) {
        const std::vector<VertexBufferComponent>& elements = layout.GetComponents();
        size_t offset = 0;
        for (uint i = 0; i < elements.size(); i++) {
            const auto& elem = elements[i];
            GL_CALL(glEnableVertexAttribArray(i));
            if (elem.flags & VertexBufferComponent::INTEGER_FLAG)
                GL_CALL(glVertexAttribIPointer(i, elem.count, (int)elem.type, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            else
                GL_CALL(glVertexAttribPointer(i, elem.count, (int)elem.type, elem.flags & elem.NORMALIZED_FLAG, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            offset += elem.count * SizeOf(elem.type);
        }
    }

    void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
        Bind();
        vb.Bind();
        AddBuffer(layout);
    }
}
