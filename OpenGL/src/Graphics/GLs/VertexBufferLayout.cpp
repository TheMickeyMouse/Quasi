#include "VertexBufferLayout.h"

namespace Graphics {
    VertexBufferLayout::VertexBufferLayout(std::initializer_list<VertexBufferComponent> comps) {
        _components.reserve(comps.size());
        for (const auto& component : comps) Push(component);
    }

    void VertexBufferLayout::Push(VertexBufferComponent comp) {
        _components.push_back(comp);
        stride += comp.count * SizeOf(comp.type);
    }

    void VertexBufferLayout::PushLayout(const VertexBufferLayout& layout) {
        for (const auto& component : layout.GetComponents()) Push(component);
    }
}
