#pragma once

#include "Utils/Type.h"

namespace Quasi::Graphics {
    using GraphicsID = u32;
    constexpr GraphicsID GraphicsNoID = 0;

    template <class G>
    class GLObject {
    public:
        GraphicsID rendererID = GraphicsNoID;

        explicit GLObject() = default;
        explicit GLObject(GraphicsID id) : rendererID(id) {}

        ~GLObject() { Destroy(); }

        GLObject(const GLObject&) = delete;
        GLObject& operator=(const GLObject&) = delete;
        GLObject(GLObject&& obj) noexcept : rendererID(obj.rendererID) { obj.rendererID = GraphicsNoID; }
        GLObject& operator=(GLObject&& obj) noexcept {
            this->~GLObject();
            new (this) GLObject(std::move(obj));
            return *this;
        }

        void Bind() const { G::BindObject(rendererID); }
        void Unbind() const { G::UnbindObject(); }

        void Destroy() { G::DestroyObject(rendererID); rendererID = GraphicsNoID; }

        bool IsNull() const { return rendererID == GraphicsNoID; }

        operator bool() const { return !IsNull(); }
        operator GraphicsID() const { return rendererID; }
    };
}
