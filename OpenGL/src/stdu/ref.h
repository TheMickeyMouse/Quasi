#pragma once
#include <optional>

namespace stdu {
    template <class T>
    struct ref {
        T* obj = nullptr;
        
        ref() : obj(nullptr) {}
        ref(T& t) : obj(&t) {}
        ref(const std::optional<T>& opt) : obj(&*opt) {}
        ref(std::nullptr_t) : obj(nullptr) {}
        ref(std::nullopt_t) : obj(nullptr) {}

        T& value() requires (!std::is_const_v<T>) { return *obj; }
        [[nodiscard]] const T& value() const { return *obj; }
        [[nodiscard]] const T& value_or(const T& none) const { return obj == nullptr ? none : *obj; }
        [[nodiscard]] bool is_null() const { return obj == nullptr; }
        [[nodiscard]] bool equals(ref r) const {
            if (is_null() && r.is_null()) return true;
            if (is_null() ^  r.is_null()) return false;
            return *r == *obj;
        }
        [[nodiscard]] bool ref_equals(ref r) const { return &*r == obj; }
        void set_ref(ref r) { obj = &*r; }
        void reset_null() { obj = nullptr; }

        T& operator*() requires (!std::is_const_v<T>) { return *obj; }
        const T& operator*() const { return *obj; }
        T* operator->() requires (!std::is_const_v<T>) { return obj; }
        const T* operator->() const { return obj; }

        bool operator==(const ref& r) const { return this->equals(r); }

        [[nodiscard]] operator bool() const { return obj != nullptr; }
        [[nodiscard]] operator const T&() const { return *obj; }
        [[nodiscard]] operator T&() { return *obj; }
    };

    template <class T> using cref = ref<const T>;
}
