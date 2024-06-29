#pragma once
#include <variant>
#include "Ref.h"
#include "Type.h"

namespace Quasi {
    namespace details {
        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };
    }

    template <class... Ts>
    struct Variant {
        std::variant<Ts...> v;

        Variant() = default;
        template <class T> requires (std::is_same_v<std::remove_cvref_t<T>, Ts> || ...)
        Variant(T&& val) : v(std::forward<T>(val)) {}

        template <class T> [[nodiscard]] bool Is() const { return std::holds_alternative<T>(v); }
        template <class T> Ref<const T> As() const { return Is<T>() ? Refer(std::get<T>(v)) : Ref<const T> { nullptr }; }
        template <class T> Ref<T> As() { return Is<T>() ? Refer(std::get<T>(v)) : Ref<T> { nullptr }; }

        template <class T> void Set(T&& val) { v = std::forward<T>(val); }

        [[nodiscard]] usize ID() const { return v.index(); }

        [[nodiscard]] auto operator<=>(const Variant&) const = default;

        template <class V>    auto Visit(V&&    visitor) const { return std::visit(std::forward<V>(visitor), v); }
        template <class V>    auto Visit(V&&    visitor)       { return std::visit(std::forward<V>(visitor), v); }
        template <class... V> auto Visit(V&&... visitor) const requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
        template <class... V> auto Visit(V&&... visitor)       requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
    };
}
