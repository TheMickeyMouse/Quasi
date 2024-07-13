#pragma once
#include <vector>

#include "GLTypeID.h"
#include "Math/Vector.h"
#include "Math/Color.h"

namespace Quasi::Graphics {
    struct VertexBufferComponent {
        GLTypeID type;
        u32 count : 24;
        u32 flags : 8;
        static constexpr byte NORMALIZED_FLAG = 1 << 0;
        static constexpr byte INTEGER_FLAG = 1 << 1;

        VertexBufferComponent() = default;
        VertexBufferComponent(GLTypeID type, uint count, bool norm = false, bool integral = false)
            : type(type), count(count), flags((integral * INTEGER_FLAG) | (norm * NORMALIZED_FLAG)) {}
        
        static VertexBufferComponent Float()  { return { GLGetTypeID<float>,  1 }; }
        static VertexBufferComponent Double() { return { GLGetTypeID<double>, 1 }; }
        static VertexBufferComponent Int()    { return { GLGetTypeID<int>,    1 }; }
        static VertexBufferComponent Uint()   { return { GLGetTypeID<uint>,   1 }; }
        static VertexBufferComponent SByte()  { return { GLGetTypeID<char>,   1 }; }
        static VertexBufferComponent Byte()   { return { GLGetTypeID<uchar>,  1 }; }
        static VertexBufferComponent Vec2()   { return { GLGetTypeID<float>,  2 }; }
        static VertexBufferComponent Vec3()   { return { GLGetTypeID<float>,  3 }; }
        static VertexBufferComponent Vec4()   { return { GLGetTypeID<float>,  4 }; }
        static VertexBufferComponent IVec2()  { return { GLGetTypeID<int>,    2 }; }
        static VertexBufferComponent IVec3()  { return { GLGetTypeID<int>,    3 }; }
        static VertexBufferComponent IVec4()  { return { GLGetTypeID<int>,    4 }; }

        template <class T> static VertexBufferComponent Type() {
            if constexpr (std::is_floating_point_v<T>) return { GLGetTypeID<T>, 1 };
            if constexpr (std::is_integral_v<T>) return { GLGetTypeID<T>, 1, false, true };
            if constexpr (Math::IVector<T> || Math::ColorLike<T>)
                return { GLGetTypeID<typename T::scalar>, T::dimension };
            return { GLTypeID::UNDEFINED, 0 };
        }
    };
    
    class VertexBufferLayout {
    private:
        Vec<VertexBufferComponent> components;
        u32 stride = 0;
    public:
        VertexBufferLayout() = default;
        VertexBufferLayout(IList<VertexBufferComponent> comps);

        template <class... Ts> static VertexBufferLayout FromTypes() {
            return { VertexBufferComponent::Type<Ts>()... };
        }

        template <class T> void Push(u32 count, bool normalized = false, bool integral = false);
        void Push(VertexBufferComponent comp);
        void PushLayout(const VertexBufferLayout& layout);

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

        [[nodiscard]] const Vec<VertexBufferComponent>& GetComponents() const { return components; }
        [[nodiscard]] u32 GetStride() const { return stride; }
    };

    template <class T>
    void VertexBufferLayout::Push(u32 count, bool normalized, bool integral) {
        constexpr GLTypeID type = GLGetTypeID<T>;
        components.emplace_back(type, count, normalized, integral);
        stride += count * SizeOf(type);
    }
}
