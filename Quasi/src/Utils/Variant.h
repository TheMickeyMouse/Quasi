#pragma once
#include <variant>
#include "Ref.h"
#include "Type.h"

namespace Quasi {


    // template <class... Ts>
    // struct Variant {
    //     std::variant<Ts...> v;
    //
    //     Variant() = default;
    //     template <class T> requires (std::is_same_v<std::remove_cvref_t<T>, Ts> || ...)
    //     Variant(T&& val) : v(std::forward<T>(val)) {}
    //
    //     template <class T> bool Is() const { return std::holds_alternative<T>(v); }
    //     template <class T> OptRef<const T> As() const { return OptRefs::DerefPtr(std::get_if<T>(&v)); }
    //     template <class T> OptRef<T> As() { return OptRefs::DerefPtr(std::get_if<T>(&v)); }
    //
    //     template <class T> void Set(T&& val) { v = std::forward<T>(val); }
    //
    //     usize ID() const { return v.index(); }
    //
    //     auto operator<=>(const Variant&) const = default;
    //
    //     template <class V>    auto Visit(V&&    visitor) const { return std::visit(std::forward<V>(visitor), v); }
    //     template <class V>    auto Visit(V&&    visitor)       { return std::visit(std::forward<V>(visitor), v); }
    //     template <class... V> auto Visit(V&&... visitor) const requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
    //     template <class... V> auto Visit(V&&... visitor)       requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
    // };
    //
    // template <class... Ts>
    // struct Text::Formatter<Variant<Ts...>> {
    //     bool AddOption() { return true; }
    //     void FormatTo(const Variant<Ts...>& var, StringOutput output) const {
    //         var.Visit(
    //             [&] (const Ts& x) {
    //                 FormatOnto(output,
    //                     "{} {{ {} }}",
    //                     Text::TypeName<Ts>(),
    //                     x
    //                 );
    //             }...
    //         );
    //     }
    // };
}
