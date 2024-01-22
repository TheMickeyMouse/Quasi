#include "VertexBufferLayout.h"

namespace Graphics {
    VertexBufferLayout::VertexBufferLayout(std::initializer_list<VertexBufferComponent> comps) {
        for (const auto& component : comps) Push(component);
    }

    void VertexBufferLayout::Push(VertexBufferComponent comp) {
        _components.push_back(comp);
        stride += comp.count * GLSizeOf(comp.type);
    }

    void VertexBufferLayout::PushLayout(const VertexBufferLayout& layout) {
        for (const auto& component : layout.GetComponents()) Push(component);
    }

    VertexBufferLayout& VertexBufferLayout::Join(VertexBufferComponent comp) {
        Push(comp);
        return *this;
    }
}
