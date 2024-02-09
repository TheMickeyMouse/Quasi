#include "VertexArray.h"

#include "../Debugging.h"

namespace Graphics {
    glID VertexArrayHandler::Create() const {
        glID id;
        GLCALL(glGenVertexArrays(1, &id));
        return id;
    }

    void VertexArrayHandler::Destroy(const glID id) const {
        GLCALL(glDeleteVertexArrays(1, &id));
    }

    void VertexArrayHandler::Bind(const glID id) const {
        GLCALL(glBindVertexArray(id));
    }

    void VertexArrayHandler::Unbind() const {
        GLCALL(glBindVertexArray(0));
    }

    void VertexArray::AddBuffer(const VertexBufferLayout& layout) {
        const std::vector<VertexBufferComponent>& elements = layout.GetComponents();
        size_t offset = 0;
        for (uint i = 0; i < elements.size(); i++) {
            const auto& elem = elements[i];
            GLCALL(glEnableVertexAttribArray(i));
            GLCALL(glVertexAttribPointer(i, elem.count, (int)elem.type, elem.normalized, layout.GetStride(), (const void*)offset));  // NOLINT(performance-no-int-to-ptr)
            offset += elem.count * SizeOf(elem.type);
        }
    }

    void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
        Bind();
        vb.Bind();
        AddBuffer(layout);
    }

    void VertexArray::AddBuffer(const DynamicVertexBuffer& vb, const VertexBufferLayout& layout) {
        Bind();
        vb.Bind();
        AddBuffer(layout);
    }
}
