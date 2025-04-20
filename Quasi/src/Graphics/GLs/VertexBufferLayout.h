#pragma once
#include <vector>

#include "GLTypeID.h"
#include "Math/Vector.h"
#include "Math/Color.h"
#include "Utils/Vec.h"

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
        
        static VertexBufferComponent Float()  { return { GetTypeIDFor<float>(),  1 }; }
        static VertexBufferComponent Double() { return { GetTypeIDFor<double>(), 1 }; }
        static VertexBufferComponent Int()    { return { GetTypeIDFor<int>(),    1 }; }
        static VertexBufferComponent Uint()   { return { GetTypeIDFor<uint>(),   1 }; }
        static VertexBufferComponent SByte()  { return { GetTypeIDFor<char>(),   1 }; }
        static VertexBufferComponent Byte()   { return { GetTypeIDFor<uchar>(),  1 }; }
        static VertexBufferComponent Vec2()   { return { GetTypeIDFor<float>(),  2 }; }
        static VertexBufferComponent Vec3()   { return { GetTypeIDFor<float>(),  3 }; }
        static VertexBufferComponent Vec4()   { return { GetTypeIDFor<float>(),  4 }; }
        static VertexBufferComponent IVec2()  { return { GetTypeIDFor<int>(),    2 }; }
        static VertexBufferComponent IVec3()  { return { GetTypeIDFor<int>(),    3 }; }
        static VertexBufferComponent IVec4()  { return { GetTypeIDFor<int>(),    4 }; }

        template <class T> static VertexBufferComponent Type() {
            if constexpr (Floating<T>) return { GetTypeIDFor<T>(), 1 };
            if constexpr (Integer<T>) return { GetTypeIDFor<T>(), 1, false, true };
            if constexpr (Math::IVector<T> || Math::ColorLike<T>)
                return { GetTypeIDFor<typename T::scalar>(), T::dimension };
            return { GLTypeID::NONE, 0 };
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

        void PushFloat()      { Push(VertexBufferComponent::Float());  }
        void PushDouble()     { Push(VertexBufferComponent::Double()); }
        void PushInt()        { Push(VertexBufferComponent::Int());    }
        void PushUInt()       { Push(VertexBufferComponent::Uint());   }
        void PushChar()       { Push(VertexBufferComponent::SByte());  }
        void PushByte()       { Push(VertexBufferComponent::Byte());   }
        void PushVector2()    { Push(VertexBufferComponent::Vec2());   }
        void PushVector3()    { Push(VertexBufferComponent::Vec3());   }
        void PushVector4()    { Push(VertexBufferComponent::Vec4());   }
        void PushVector2Int() { Push(VertexBufferComponent::IVec2());  }
        void PushVector3Int() { Push(VertexBufferComponent::IVec3());  }
        void PushVector4Int() { Push(VertexBufferComponent::IVec4());  }

        const Vec<VertexBufferComponent>& GetComponents() const { return components; }
        u32 GetStride() const { return stride; }
    };

    template <class T>
    void VertexBufferLayout::Push(u32 count, bool normalized, bool integral) {
        GLTypeID type = GetTypeIDFor<T>();
        components.Push({ type, count, normalized, integral });
        stride += count * type->typeSize;
    }
}
