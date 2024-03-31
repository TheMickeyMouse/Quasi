#pragma once
#include <type_traits>

#include "NumTypes.h"
#include "core.h"
#include "stdu/types.h"

namespace Graphics {
    using glID = uint;
    constexpr glID GL_NULL = 0;

    template <class H>
    struct GLObjectHandler {
        [[nodiscard]] glID Create() const { return ((const H*)this)->Create(); }
        void Destroy(glID id) const { ((const H*)this)->Destroy(id); }
        void Bind(glID id) const { ((const H*)this)->Bind(id); }
        void Unbind() const { ((const H*)this)->Unbind(); }
    };

    template <class H> requires std::is_base_of_v<GLObjectHandler<H>, H>
    class GLObject {
    public:
        [[msvc::no_unique_address]] H handler {};
        glID rendererID = GL_NULL;

        explicit GLObject() = default;
        GLObject(stdu::empty) : rendererID(handler.Create()) {}
        ~GLObject() { Delete(); }

        GLObject(const GLObject&) = delete;
        GLObject& operator=(const GLObject&) = delete;
        GLObject(GLObject&& obj) noexcept : handler(obj.handler), rendererID(obj.rendererID) {
            obj.rendererID = GL_NULL;
        }
        GLObject& operator=(GLObject&& obj) noexcept {
            rendererID = obj.rendererID; obj.rendererID = GL_NULL;
            handler = std::move(obj.handler);
            return *this;
        }

        void Bind() const { handler.Bind(rendererID); }
        void Unbind() const { handler.Unbind(); }

        void Delete() { handler.Destroy(rendererID); rendererID = GL_NULL; }

        [[nodiscard]] bool IsNull() const { return rendererID == GL_NULL; }

        [[nodiscard]] const H& Handler() const { return handler; }
        H& Handler() { return handler; }

        [[nodiscard]] operator glID() const { return rendererID; }
    };

    enum class BufferType {
        VERTEX = 0x8892,
        INDEX = 0x8893,
    };

    template <BufferType T>
    struct BufferHandler : GLObjectHandler<BufferHandler<T>> {
        [[nodiscard]] OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };
}
