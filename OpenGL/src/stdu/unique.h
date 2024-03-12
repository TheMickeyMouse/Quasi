#pragma once
#include <algorithm>

namespace stdu {
    // same as std::unique_ptr, but not just for pointers
    template <class T, class H>
    requires requires(H h, T t) { { h() } -> std::convertible_to<T>; { h(t) } -> std::same_as<void>; }
    struct unique {
        T val;
        [[no_unique_address]] H handler {};

        unique() : val(H {} ()) {}
        unique(T t) : val(t) {}
        unique(T t, H hand) : val(t), handler(hand) {}
        unique(unique&& u) noexcept : val(u.val), handler(u.handler) { u.val = u.handler(); }
        unique(const unique&) = delete;

        ~unique() { handler(val); }

        unique& operator=(unique&& u) noexcept { handler(val); val = u.val; u.val = u.handler(); return *this; }
        unique& operator=(const unique& u) = delete;

        T release() { T v = val; val = handler(); return v; }
        void reset(T newVal) { handler(val); val = newVal; }
        void swap(unique& u) noexcept { std::swap(val, u.val); /* there's probably no need to swap handler */ }

        T get() const { return val; }
        T& get() { return val; }
        H get_handler() const { return handler; }
        H& get_handler() { return handler; }

        bool is_null() const { return val == handler(); }

        [[nodiscard]] operator bool() const { return !is_null(); }
        [[nodiscard]] operator T() const { return val; }

        T operator*() const { return get(); }
        T& operator*() { return get(); }
        const T* operator->() const { return &val; }
        T* operator->() { return &val; }
    };

    template <class T, class H, class... Args>
    unique<T, H> make_unique(Args... args) {
        return unique<T, H>(T { args... });
    }
}
