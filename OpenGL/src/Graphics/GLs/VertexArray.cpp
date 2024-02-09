#include "VertexArray.h"

#include "../Debugging.h"

namespace Graphics {
    VertexArray::VertexArray() {
        GLCALL(glGenVertexArrays(1, &rendererID));
    }

    VertexArray::~VertexArray() {
        GLCALL(glDeleteVertexArrays(1, &rendererID));
    }

    void VertexArray::Transfer(VertexArray& dest, VertexArray&& from) {
        dest.rendererID = from.rendererID;
        from.rendererID = 0;
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

    void VertexArray::Bind() const {
        GLCALL(glBindVertexArray(rendererID));
    }

    void VertexArray::Unbind() const {
        GLCALL(glBindVertexArray(0));
    }
}
