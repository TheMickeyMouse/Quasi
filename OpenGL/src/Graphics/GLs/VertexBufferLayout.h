#pragma once
#include <vector>

#include "GLTypeID.h"
#include "opengl.h"

#include "NumTypes.h"
#include "Vector.h"

namespace Graphics {
    struct VertexBufferComponent {
        GLTypeID type;
        uint count;
        uchar flags;
        static constexpr uchar NORMALIZED_FLAG = 1;
        static constexpr uchar INTEGER_FLAG = 2;

        VertexBufferComponent() = default;
        VertexBufferComponent(GLTypeID type, uint count, bool norm = false, bool integral = false)
            : type(type), count(count), flags((integral * INTEGER_FLAG) | (norm * NORMALIZED_FLAG)) {}
        
        static VertexBufferComponent Float()  { return { GLTypeIDOf<float>,  1 }; }
        static VertexBufferComponent Double() { return { GLTypeIDOf<double>, 1 }; }
        static VertexBufferComponent Int()    { return { GLTypeIDOf<int>,    1 }; }
        static VertexBufferComponent Uint()   { return { GLTypeIDOf<uint>,   1 }; }
        static VertexBufferComponent SByte()  { return { GLTypeIDOf<char>,   1 }; }
        static VertexBufferComponent Byte()   { return { GLTypeIDOf<uchar>,  1 }; }
        static VertexBufferComponent Vec2()   { return { GLTypeIDOf<float>,  2 }; }
        static VertexBufferComponent Vec3()   { return { GLTypeIDOf<float>,  3 }; }
        static VertexBufferComponent Vec4()   { return { GLTypeIDOf<float>,  4 }; }
        static VertexBufferComponent IVec2()  { return { GLTypeIDOf<int>,    2 }; }
        static VertexBufferComponent IVec3()  { return { GLTypeIDOf<int>,    3 }; }
        static VertexBufferComponent IVec4()  { return { GLTypeIDOf<int>,    4 }; }

        template <class T> static VertexBufferComponent Type() {
            if constexpr (std::is_floating_point_v<T>) return { GLTypeIDOf<T>, 1 };
            if constexpr (std::is_integral_v<T>) return { GLTypeIDOf<T>, 1, false, true };
            if constexpr (Maths::is_vec_v<T> || Maths::is_color_v<T>)
                return { GLTypeIDOf<typename T::scalar>, T::dimension };
            return { GLTypeID::UNDEFINED, 0 };
        }
    };
    
    class VertexBufferLayout {
    private:
        std::vector<VertexBufferComponent> _components;
        uint stride = 0;
    public:
        OPENGL_API VertexBufferLayout() = default;
        OPENGL_API VertexBufferLayout(std::initializer_list<VertexBufferComponent> comps);

        template <class T> void Push(uint count, bool normalized = false, bool integral = false);
        OPENGL_API void Push(VertexBufferComponent comp);
        OPENGL_API void PushLayout(const VertexBufferLayout& layout);

        OPENGL_API VertexBufferLayout& Join(VertexBufferComponent comp);

#define VBE VertexBufferComponent::
        
        void PushFloat()      { Push(VBE Float());  }
        void PushDouble()     { Push(VBE Double()); }
        void PushInt()        { Push(VBE Int());    }
        void PushUInt()       { Push(VBE Uint());   }
        void PushChar()       { Push(VBE SByte());  }
        void PushByte()       { Push(VBE Byte());   }
        void PushVector2()    { Push(VBE Vec2());   }
        void PushVector3()    { Push(VBE Vec3());   }
        void PushVector4()    { Push(VBE Vec4());   }
        void PushVector2Int() { Push(VBE IVec2());  }
        void PushVector3Int() { Push(VBE IVec3());  }
        void PushVector4Int() { Push(VBE IVec4());  }

#undef VBE

        [[nodiscard]] const std::vector<VertexBufferComponent>& GetComponents() const { return _components; }
        [[nodiscard]] uint GetStride() const { return stride; }
    };

    template <class T>
    void VertexBufferLayout::Push(uint count, bool normalized, bool integral) {
        constexpr GLTypeID type = GLTypeIDOf<T>;
        _components.emplace_back(type, count, normalized, integral);
        stride += count * SizeOf(type);
    }
}
