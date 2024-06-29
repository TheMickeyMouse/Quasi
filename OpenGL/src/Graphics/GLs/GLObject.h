#pragma once

#include "Utils/Type.h"

namespace Quasi::Graphics {
    using GraphicsID = u32;
    constexpr GraphicsID GraphicsNoID = 0;

    template <class H> concept GLObjectHandler = requires (H h, GraphicsID id) {
        { h.Create()    } -> std::same_as<GraphicsID>;
        { h.Destroy(id) } -> std::same_as<void>;
        { h.Bind(id)    } -> std::same_as<void>;
        { h.Unbind()    } -> std::same_as<void>;
    };

    template <GLObjectHandler H>
    class GLObject {
    public:
        GraphicsID rendererID = GraphicsNoID;
        [[no_unique_address]] H handler {};

        explicit GLObject() = default;
        void Create() { rendererID = handler.Create(); }
        ~GLObject() { Delete(); }

        GLObject(const GLObject&) = delete;
        GLObject& operator=(const GLObject&) = delete;
        GLObject(GLObject&& obj) noexcept : rendererID(obj.rendererID), handler(std::move(obj.handler)) {
            obj.rendererID = GraphicsNoID;
        }
        GLObject& operator=(GLObject&& obj) noexcept {
            this->~GLObject();
            new (this) GLObject(std::move(obj));
            return *this;
        }

        void Bind() const { handler.Bind(rendererID); }
        void Unbind() const { handler.Unbind(); }

        void Delete() { handler.Destroy(rendererID); rendererID = GraphicsNoID; }

        [[nodiscard]] bool IsNull() const { return rendererID == GraphicsNoID; }

        H& Handler() { return handler; }
        [[nodiscard]] const H& Handler() const { return handler; }

        [[nodiscard]] operator bool() const { return !IsNull(); }
        [[nodiscard]] operator GraphicsID() const { return rendererID; }
    };

    enum class BufferType {
        VERTEX = 0x8892,
        INDEX = 0x8893,
    };

    template <BufferType T>
    struct BufferHandler {
        [[nodiscard]] GraphicsID Create() const;
        void Destroy(GraphicsID id) const;
        void Bind(GraphicsID id) const;
        void Unbind() const;
    };
}
