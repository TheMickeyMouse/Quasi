#include "VertexBufferLayout.h"

namespace Quasi::Graphics {
    VertexBufferLayout::VertexBufferLayout(IList<VertexBufferComponent> comps) {
        components.reserve(comps.size());
        for (const auto& component : comps) Push(component);
    }

    void VertexBufferLayout::Push(VertexBufferComponent comp) {
        components.push_back(comp);
        stride += comp.count * SizeOf(comp.type);
    }

    void VertexBufferLayout::PushLayout(const VertexBufferLayout& layout) {
        for (const auto& component : layout.GetComponents()) Push(component);
    }
}
