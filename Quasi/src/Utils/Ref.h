#pragma once
#include "Memory.h"
#include "Option.h"
#include "Type.h"

namespace Quasi {
    template <class> struct Ref;
    template <class> struct OptRef;

    /*
     * An interface that describes ref classes.
     * Must implement:
     * const? T& ValueImpl() const?;
     */
    template <class T, class Super>
    struct RefProxy {
        Super& super() { return *static_cast<Super*>(this); }
        const Super& super() const { return *static_cast<const Super*>(this); }

        T& Value() requires IsMut<T> { return super().ValueImpl(); }
        const T& Value() const { return super().ValueImpl(); }
        T* Address() requires IsMut<T> { return &Value(); }
        const T* Address() const { return &Value(); }

        T& ValueImpl() requires IsMut<T> = delete;
        const T& ValueImpl() const = delete;

        [[nodiscard]] operator const T&() const { return Value(); }
        [[nodiscard]] operator T&() { return Value(); }

        T& operator*() requires IsMut<T> { return Value(); }
        const T& operator*() const { return Value(); }
        T* operator->() requires IsMut<T> { return Address(); }
        const T* operator->() const { return Address(); }

        Ref<T> AsRef() requires IsMut<T> { return Ref<T>::Deref(Address()); }
        Ref<const T> AsRef() const { return Ref<const T>::Deref(Address()); }
    };

    template <class T>
    struct Ref : RefProxy<T, Ref<T>> {
        T* obj;
    private:
        Ref(T* t) : obj(t) {}
    public:
        Ref(T& t) : obj(&t) {}

        using ConstRef = Ref<const T>;

        static Ref Deref(T* t) { return { t }; }

        T& ValueImpl() { return *obj; }
        [[nodiscard]] const T& ValueImpl() const { return *obj; }

        [[nodiscard]] bool RefEquals(Ref r) const { return r.obj == obj; }
        void SetRef(Ref r) { obj = &*r; }

        bool operator==(ConstRef r) const { return this->Equals(r); }

        [[nodiscard]] Ref<RemConst<T>> AsMut() requires IsConst<T> { return { (RemConst<T>*)obj }; }
        [[nodiscard]] Ref<const T> AsConst() const requires IsMut<T> { return { (const T*)obj }; }

        operator ConstRef() const { return AsConst(); }
        explicit operator Ref<RemConst<T>>() { return AsMut(); }
        template <Extends<T> Base> requires DifferentTo<T, Base> operator OptRef<const Base>() const { return obj; }
        template <Extends<T> Base> requires DifferentTo<T, Base> && IsMut<Base> operator OptRef<Base>() { return obj; }

        template <Extends<T> Derived>
        OptRef<const Derived> As() const { return dynamic_cast<const Derived*>(obj); }
        template <Extends<T> Derived> requires IsMut<T>
        OptRef<Derived> As() { return dynamic_cast<Derived*>(obj); }

        template <class Re> Ref<Re> TransmuteAs() { return Memory::TransmutePtr<Re>(obj); }
        template <class Re> Ref<const Re> TransmuteAs() const { return Memory::TransmutePtr<const Re>(obj); }

        template <class U> friend struct Ref;
    };

    template <class T>
    struct OptRef : NullableProxy<T&, OptRef<T>>, RefProxy<T, OptRef<T>> {
    private:
        T* obj = nullptr;
        OptRef(T* t) : obj(t) {}
    public:
        OptRef() : obj(nullptr) {}
        OptRef(Nullptr) : obj(nullptr) {}
        OptRef(T& t) : obj(&t) {}
        OptRef(Ref<T> t) : obj(t.Address()) {}

        using RefProxy<T, OptRef>::operator->;
        using RefProxy<T, OptRef>::operator*;

        using ConstRef = OptRef<const T>;

        static OptRef Deref(T* t) { return { t }; }
        static OptRef SomeImpl(const T& value) requires IsConst<T> { return { value }; }
        static OptRef SomeImpl(T& value) { return { value }; }
        static OptRef NoneImpl() { return { nullptr }; }

        T& ValueImpl() { return *obj; }
        [[nodiscard]] const T& ValueImpl() const { return *obj; }

        [[nodiscard]] bool HasValueImpl() const { return obj; }
        T& UnwrapImpl() const { return *obj; }

        void SetNullImpl() { obj = nullptr; }
        void SetImpl(T& v) { obj = &v; }

        [[nodiscard]] bool Equals(OptRef r) const {
            return this->IsNull() ? r.IsNull() : r.HasValue() && *r == *obj;
        }
        [[nodiscard]] bool RefEquals(OptRef r) const { return r.obj == obj; }
        void SetRef(OptRef r) { obj = r.Address(); }
        void SetNull() { obj = nullptr; }

        bool operator==(ConstRef r) const { return this->Equals(r); }

        [[nodiscard]] OptRef<RemConst<T>> AsMut() requires IsConst<T> { return { (RemConst<T>*)obj }; }
        [[nodiscard]] OptRef<const T> AsConst() const requires IsMut<T> { return { (const T*)obj }; }

        operator ConstRef() const { return AsConst(); }
        explicit operator OptRef<RemConst<T>>() { return AsMut(); }
        template <Extends<T> Base> requires DifferentTo<T, Base> operator OptRef<const Base>() const { return obj; }
        template <Extends<T> Base> requires DifferentTo<T, Base> && IsMut<Base> operator OptRef<Base>() { return obj; }

        template <Extends<T> Derived>
        OptRef<const Derived> As() const { return dynamic_cast<const Derived*>(obj); }
        template <Extends<T> Derived> requires IsMut<T>
        OptRef<Derived> As() { return dynamic_cast<Derived*>(obj); }

        template <class Re> OptRef<Re> TransmuteAs() { return Memory::TransmutePtr<Re>(obj); }
        template <class Re> OptRef<const Re> TransmuteAs() const { return Memory::TransmutePtr<const Re>(obj); }

        template <class U> friend struct OptRef;
    };

    template <class T> struct RemoveQRef : std::type_identity<T> {};
    template <class T> struct RemoveQRef<Ref<T>> : std::type_identity<T> {};
    template <class T> struct RemoveQRef<const Ref<T>> : std::type_identity<const T> {};
    template <class T> struct RemoveQRef<OptRef<T>> : std::type_identity<T> {};
    template <class T> struct RemoveQRef<const OptRef<T>> : std::type_identity<const T> {};

    template <class T> Ref<typename RemoveQRef<T>::type> Refer(T& val) { return { val }; }
    template <class T> OptRef<typename RemoveQRef<T>::type> SomeRef(T& val) { return { val }; }
    template <class T> OptRef<T> DerefPtr(T* val) { return OptRef<T>::Deref(val); }
}
