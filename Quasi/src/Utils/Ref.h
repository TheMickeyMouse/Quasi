#pragma once
#include <optional>

namespace Quasi {
    template <class T>
    struct Ref {
        T* obj = nullptr;
    private:
        Ref(T* t) : obj(t) {}
    public:
        Ref() : obj(nullptr) {}
        Ref(T& t) : obj(&t) {}
        Ref(std::nullptr_t) : obj(nullptr) {}
        Ref(std::nullopt_t) : obj(nullptr) {}

        using ConstRef = Ref<const T>;

        static Ref From(T* t) { return { t }; }

        T& Value() requires (!std::is_const_v<T>) { return *obj; }
        [[nodiscard]] const T& Value() const { return *obj; }
        T& Value() { return *obj; }
        [[nodiscard]] const T& ValueOr(const T& none) const { return obj == nullptr ? none : *obj; }
        T& ValueOr(T& none) { return obj == nullptr ? none : *obj; }
        [[nodiscard]] ConstRef ValueOr(ConstRef none) const { return obj == nullptr ? none : Refer(*obj); }
        Ref ValueOr(Ref none) { return obj == nullptr ? none : Refer(*obj); }

        [[nodiscard]] bool HasValue() const { return obj; }
        [[nodiscard]] bool IsNull() const { return obj == nullptr; }
        [[nodiscard]] bool Equals(Ref r) const {
            return IsNull() ? r.IsNull() : r.HasValue() && *r == *obj;
        }
        [[nodiscard]] bool RefEquals(Ref r) const { return r.obj == obj; }
        void SetRef(Ref r) { obj = &*r; }
        void SetNull() { obj = nullptr; }

        T& operator*() requires (!std::is_const_v<T>) { return *obj; }
        const T& operator*() const { return *obj; }
        T* operator->() requires (!std::is_const_v<T>) { return obj; }
        const T* operator->() const { return obj; }

        bool operator==(ConstRef r) const { return this->Equals(r); }

        [[nodiscard]] operator bool() const { return obj != nullptr; }
        [[nodiscard]] operator const T&() const { return *obj; }
        [[nodiscard]] operator T&() { return *obj; }
        [[nodiscard]] const T* Address() const { return obj; }
        T* Address() { return obj; }

        [[nodiscard]] Ref<std::remove_const_t<T>> AsMut() requires std::is_const_v<T> { return { (std::remove_const_t<T>*)obj }; }
        [[nodiscard]] Ref<const T> AsConst() const requires (!std::is_const_v<T>) { return { (const T*)obj }; }

        operator ConstRef() const { return AsConst(); }
        explicit operator Ref<std::remove_const_t<T>>() { return AsMut(); }
        template <class Base> requires (!std::is_same_v<Base, T>) && std::is_base_of_v<Base, T> operator Ref<const Base>() const { return obj; }
        template <class Base> requires (!std::is_same_v<Base, T>) && std::is_base_of_v<Base, T> && (!std::is_const_v<Base>) operator Ref<Base>() { return obj; }

        template <class Derived> requires std::is_base_of_v<T, Derived>
        Ref<const Derived> As() const { return dynamic_cast<const Derived*>(obj); }
        template <class Derived> requires std::is_base_of_v<T, Derived> && (!std::is_const_v<T>)
        Ref<Derived> As() { return dynamic_cast<Derived*>(obj); }

        template <class Re>
        Ref<Re> TransmuteAs() { return reinterpret_cast<Re*>(obj); }
        template <class Re>
        Ref<const Re> TransmuteAs() const { return reinterpret_cast<const Re*>(obj); }

        T& Assert();
        [[nodiscard]] const T& Assert() const;
        template <class Asrt> T& Assert(Asrt&& assertfn);
        template <class Asrt> [[nodiscard]] const T& Assert(Asrt&& assertfn) const;

        static Ref Deref(T* val) { return { val }; }

        template <class U> friend struct Ref;
    };

    template <class T> using CRef = Ref<const T>;

    template <class T> struct RemoveQRef : std::type_identity<T> {};
    template <class T> struct RemoveQRef<Ref<T>> : std::type_identity<T> {};
    template <class T> struct RemoveQRef<const Ref<T>> : std::type_identity<const T> {};

    template <class T> Ref<typename RemoveQRef<T>::type> Refer(T& val) { return { val }; }
    template <class T> Ref<T> DerefPtr(T* val) { return Ref<T>::Deref(val); }
}
