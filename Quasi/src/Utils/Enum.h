#pragma once
#include "Macros.h"
#include "Type.h"
#include "Ref.h"
#include <algorithm>

namespace Quasi {
    enum EnumTraits {
        NONE = 0,
        FLAGS = 1 << 0,
        ARITH = 1 << 1,
        NULLABLE = 1 << 2,
        COMPARABLE = 1 << 3,
    };

    template <class E, int TT = NONE>
    struct Enum {
        using InternalData = E;
        static constexpr bool IS_FLAG     = TT & FLAGS;
        static constexpr bool IS_ARITH    = TT & ARITH;
        static constexpr bool IS_NULLABLE = TT & NULLABLE;
        static constexpr bool IS_CMP      = TT & COMPARABLE;
        std::conditional_t<IS_FLAG, u32, const E*> value = 0;

        Enum() { SetDefault(); }
        Enum(const E& e) requires (!IS_FLAG) : value(&e) {}
        Enum(const E& e) requires IS_FLAG : value(e._VALUE) {}
        Enum(IList<Ref<const E>> flags) requires IS_FLAG : value(0) { AddFlags(flags); }

        void SetDefault() {
            if      constexpr (IS_NULLABLE) value = &E::NONE;
            else if constexpr (IS_FLAG) value = 0;
            else value = Min();
        }

        [[nodiscard]] u32 Ord() const {
            if constexpr (IS_FLAG) return value;
            else return value - E::VALUES.data();
        }

        static Enum FromOrd(u32 i) requires (!IS_FLAG) { return E::VALUES[i]; }
        static Enum FromBitflag(u32 i) requires IS_FLAG { Enum e; e.value = i; return e; }
        template <class T> static Enum Find(T E::* property, const T& searcher) {
            if constexpr (IS_NULLABLE) if ((*Null()).*property == searcher) return Null();
            auto it = std::find_if(Values().begin(), Values().end(), [=](const E& e) { return e.*property == searcher; });
            return it == Values().end() ? Null() : *it;
        }

        void AddFlag(const E& e) requires IS_FLAG { value |= e._VALUE; }
        void AddFlags(IList<Ref<const E>> flags) requires IS_FLAG {
            for (const auto f : flags) AddFlag(f);
        }

        static constexpr auto& Values() { return E::VALUES; }
        static constexpr u32 Num() { return E::NUM; }
        static constexpr Enum Min() { return E::VALUES[0]; }
        static constexpr Enum Max() { return E::VALUES.back(); }
        static constexpr Enum Null() { return E::NONE; }

        [[nodiscard]] const E* operator->() const requires (!IS_FLAG) { return value; }
        [[nodiscard]] const E& operator*()  const requires (!IS_FLAG) { return *value; }

        operator bool() const requires IS_NULLABLE { return value != Null().value; }
        explicit operator u32() const { return Ord(); }

#pragma region Operators
        Enum operator+() const requires IS_ARITH { return *this; }
        Enum operator+(Enum other) const requires IS_ARITH { return FromOrd(Ord() + other.Ord()); }
        Enum operator-(Enum other) const requires IS_ARITH { return FromOrd(Ord() - other.Ord()); }
        Enum operator+(u32 other)  const requires IS_ARITH { return FromOrd(Ord() + other); }
        Enum operator-(u32 other)  const requires IS_ARITH { return FromOrd(Ord() - other); }
        Enum operator*(u32 other)  const requires IS_ARITH { return FromOrd(Ord() * other); }
        Enum operator/(u32 other)  const requires IS_ARITH { return FromOrd(Ord() / other); }
        Enum operator%(Enum other) const requires IS_ARITH { return FromOrd(Ord() % other.Ord()); }
        Enum operator%(u32 other)  const requires IS_ARITH { return FromOrd(Ord() % other); }
        Enum& operator+=(Enum other) requires IS_ARITH { value += other.Ord(); return *this; }
        Enum& operator-=(Enum other) requires IS_ARITH { value += other.Ord(); return *this; }
        Enum& operator+=(u32 other)  requires IS_ARITH { value += other;       return *this; }
        Enum& operator-=(u32 other)  requires IS_ARITH { value += other;       return *this; }
        Enum& operator*=(u32 other)  requires IS_ARITH { value += other;       return *this; }
        Enum& operator/=(u32 other)  requires IS_ARITH { value += other;       return *this; }
        Enum& operator%=(Enum other) requires IS_ARITH { value += other.Ord(); return *this; }
        Enum& operator%=(u32 other)  requires IS_ARITH { value += other;       return *this; }
        bool operator< (Enum other) const requires IS_CMP { return Ord() <  other.Ord(); }
        bool operator<=(Enum other) const requires IS_CMP { return Ord() <= other.Ord(); }
        bool operator> (Enum other) const requires IS_CMP { return Ord() >  other.Ord(); }
        bool operator>=(Enum other) const requires IS_CMP { return Ord() >= other.Ord(); }
        bool operator==(Enum other) const { return value == other.value; }
        bool operator!=(Enum other) const { return value != other.value; }

        Enum operator~()           const requires IS_FLAG  { return FromBitflag(~value); }
        Enum operator&(Enum other) const requires IS_FLAG  { return FromBitflag(value & other.value); }
        Enum operator|(Enum other) const requires IS_FLAG  { return FromBitflag(value | other.value); }
        Enum operator^(Enum other) const requires IS_FLAG  { return FromBitflag(value ^ other.value); }
        Enum operator&(u32 other)  const requires IS_FLAG  { return FromBitflag(value & other); }
        Enum operator|(u32 other)  const requires IS_FLAG  { return FromBitflag(value | other); }
        Enum operator^(u32 other)  const requires IS_FLAG  { return FromBitflag(value ^ other); }
        Enum operator<<(u32 other) const requires IS_FLAG  { return FromBitflag(value << other); }
        Enum operator>>(u32 other) const requires IS_FLAG  { return FromBitflag(value >> other); }
        Enum& operator&=(Enum other) requires IS_FLAG  { value &= other.value; return *this; }
        Enum& operator|=(Enum other) requires IS_FLAG  { value |= other.value; return *this; }
        Enum& operator^=(Enum other) requires IS_FLAG  { value ^= other.value; return *this; }
        Enum& operator&=(u32 other)  requires IS_FLAG  { value &= other;       return *this; }
        Enum& operator|=(u32 other)  requires IS_FLAG  { value |= other;       return *this; }
        Enum& operator^=(u32 other)  requires IS_FLAG  { value ^= other;       return *this; }
        Enum& operator<<=(u32 other) requires IS_FLAG  { value <<= other;      return *this; }
        Enum& operator>>=(u32 other) requires IS_FLAG  { value >>= other;      return *this; }
#pragma endregion
    };

    template <class T> concept IEnum = requires (T t) {
        Enum { t }; // neat trick i learnt here: https://stackoverflow.com/a/54182690
    };
}

#define Q_DEFINE_ENUM(ENAME, V, TRAITS, ...) \
        Quasi::u32 _VALUE; \
        using __THIS_ENUM = ENAME##Data; \
        static constexpr Quasi::u32 __COUNTING_VALUE_BEGIN = Q_COUNTER() + 1; \
        Q_CAT(Q_E_GV_1 V, _END) \
        static constexpr Quasi::u32 NUM = Q_COUNTER() - __COUNTING_VALUE_BEGIN; \
        static const Quasi::Array<ENAME##Data, NUM> VALUES; \
        Q_IF(Q_HAS_ARGS(__VA_ARGS__), static const ENAME##Data NONE; ) \
    }; \
    inline const Quasi::Array<ENAME##Data, ENAME##Data::NUM> ENAME##Data::VALUES = { Q_UNARY(Q_DEFER(Q_REMOVE_FIRST)(Q_CAT(Q_E_PN_1 V, _END))) }; \
    Q_IF(Q_HAS_ARGS(__VA_ARGS__), inline const ENAME##Data ENAME##Data::NONE = { Q_UNARY __VA_ARGS__, ~0u }; ) \
    struct ENAME : Quasi::Enum<ENAME##Data, TRAITS> { \
        using __THIS_ENUM = ENAME; \
        static constexpr Quasi::u32 __COUNTING_VALUE_BEGIN = Q_COUNTER() + 1; \
        Q_CAT(Q_E_GVR_1 V, _END) \
        Q_IF(Q_HAS_ARGS(__VA_ARGS__), inline static const InternalData& NONE = ENAME##Data::NONE; ) \
        using Enum::Enum; \
        ENAME(Enum e) : Enum(e) {} \


#define Q_E_GV_1(N, X) static __THIS_ENUM Make##N() { return (__THIS_ENUM { Q_UNARY X Q_DEFER(Q_COMMA)() Q_COUNTER() - __COUNTING_VALUE_BEGIN }); } Q_E_GV_2
#define Q_E_GV_2(N, X) static __THIS_ENUM Make##N() { return (__THIS_ENUM { Q_UNARY X Q_DEFER(Q_COMMA)() Q_COUNTER() - __COUNTING_VALUE_BEGIN }); } Q_E_GV_1
#define Q_E_GV_2_END
#define Q_E_GV_1_END

#define Q_E_GVR_1(N, X) inline static const InternalData& N = InternalData::VALUES[Q_COUNTER() - __COUNTING_VALUE_BEGIN]; Q_E_GVR_2
#define Q_E_GVR_2(N, X) inline static const InternalData& N = InternalData::VALUES[Q_COUNTER() - __COUNTING_VALUE_BEGIN]; Q_E_GVR_1
#define Q_E_GVR_2_END
#define Q_E_GVR_1_END

#define Q_E_PN_1(N, X) Q_DEFER(Q_COMMA)() Make##N() Q_E_PN_2
#define Q_E_PN_2(N, X) Q_DEFER(Q_COMMA)() Make##N() Q_E_PN_1
#define Q_E_PN_2_END
#define Q_E_PN_1_END

namespace Quasi {
    template <IEnum E, class T>
    struct EnumMap {
        Array<T, E::Num()> arr;

        constexpr EnumMap() = default;
        constexpr EnumMap(IList<std::pair<E, T>> pairs) {
            for (const auto& [e, t] : pairs) arr[e.Ord()] = t;
        }

        [[nodiscard]] const T& operator[](E e) const { return arr[e.Ord()]; }
        T& operator[](E e) { return arr[e.Ord()]; }
    };
}
