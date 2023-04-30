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

        T& value() { return *obj; }
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

        T& operator*() { return *obj; }
        const T& operator*() const { return *obj; }
        T* operator->() { return obj; }
        const T* operator->() const { return obj; }

        bool operator==(const ref& r) const { return this->equals(r); }

        operator bool() { return obj != nullptr; }
        operator T() { return *obj; }
    };

    template <class T>
    struct cref {
        const T* obj = nullptr;
        
        cref() : obj(nullptr) {}
        cref(const T& t) : obj(&t) {}
        cref(const std::optional<T>& opt) : obj(&*opt) {}
        cref(std::nullptr_t) : obj(nullptr) {}
        cref(std::nullopt_t) : obj(nullptr) {}
        cref(ref<T> r) : obj(*r) {}
        
        [[nodiscard]] const T& value() const { return *obj; }
        [[nodiscard]] const T& value_or(const T& none) const { return obj == nullptr ? none : *obj; }
<<<<<<< HEAD
        [[nodiscard]] bool is_null() const { return obj == nullptr; }
=======
        [[nodiscard]] bool is_null() const { return obj != nullptr; }
>>>>>>> b9abc8a1ada952e8f68ca326d5cbb380117fbd4e
        [[nodiscard]] bool equals(cref r) const {
            if (is_null() && r.is_null()) return true;
            if (is_null() ^  r.is_null()) return false;
            return *r == *obj;
        }
        [[nodiscard]] bool ref_equals(cref r) const { return &*r == obj; }
        void set_ref(cref r) { obj = &*r; }
        void reset_null() { obj = nullptr; }
        
        const T& operator*() const { return *obj; }
        const T* operator->() const { return obj; }

        bool operator==(const cref& r) const { return this->equals(r); }

        operator bool() { return obj != nullptr; }
        operator T() { return *obj; }
    };
}
