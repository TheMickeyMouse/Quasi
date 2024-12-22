#pragma once
#include <variant>
#include "Ref.h"
#include "Type.h"
#include "Format.h"

namespace Quasi {
    namespace details {
        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };

        template <class... Ts>
        overloaded(Ts&&...) -> overloaded<Ts...>;
    }

    template <class... Ts>
    struct Variant {
        std::variant<Ts...> v;

        Variant() = default;
        template <class T> requires (std::is_same_v<std::remove_cvref_t<T>, Ts> || ...)
        Variant(T&& val) : v(std::forward<T>(val)) {}

        template <class T> bool Is() const { return std::holds_alternative<T>(v); }
        template <class T> OptRef<const T> As() const { return OptRefs::DerefPtr(std::get_if<T>(&v)); }
        template <class T> OptRef<T> As() { return OptRefs::DerefPtr(std::get_if<T>(&v)); }

        template <class T> void Set(T&& val) { v = std::forward<T>(val); }

        usize ID() const { return v.index(); }

        auto operator<=>(const Variant&) const = default;

        template <class V>    auto Visit(V&&    visitor) const { return std::visit(std::forward<V>(visitor), v); }
        template <class V>    auto Visit(V&&    visitor)       { return std::visit(std::forward<V>(visitor), v); }
        template <class... V> auto Visit(V&&... visitor) const requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
        template <class... V> auto Visit(V&&... visitor)       requires (sizeof...(V) >= 2) { return std::visit(details::overloaded { std::forward<V>(visitor)... }, v); }
    };

    template <class... Ts>
    struct Text::Formatter<Variant<Ts...>> {
        bool AddOption() { return true; }
        void FormatTo(const Variant<Ts...>& var, StringOutput output) const {
            var.Visit(
                [&] (const Ts& x) {
                    FormatOnto(output,
                        "{} {{ {} }}",
                        Text::TypeName<Ts>(),
                        x
                    );
                }...
            );
        }
    };

    template <class T, class... Ts> union Union;

    template <class T, class... Ts>
    union Union {
        T first;
        Union<Ts...> rest;

        template <class U> static constexpr bool IsApartOf = std::is_same_v<T, std::remove_cvref_t<U>> || (std::is_same_v<Ts, std::remove_cvref_t<U>> || ...);

        Union(const T& t) : first(t) {}
        Union(T&& t) : first(std::move(t)) {}
        template <class U> requires IsApartOf<U>
        Union(U&& t) : rest(std::forward<U>(t)) {}
        ~Union() {}

        Union(const Union&) = delete;
        Union& operator=(const Union&) = delete;

        template <class U> requires IsApartOf<U> const U& As() const {
            if constexpr (std::is_same_v<T, std::remove_cvref_t<U>>) return first;
            else return rest.template As<U>();
        }
        template <class U> requires IsApartOf<U> U& As() {
            if constexpr (std::is_same_v<T, std::remove_cvref_t<U>>) return first;
            else return rest.template As<U>();
        }
        template <class U> requires IsApartOf<U> U&& MoveAs() && {
            if constexpr (std::is_same_v<T, std::remove_cvref_t<U>>) return std::move(first);
            else return std::move(rest.template As<U>());
        }

        template <class U> requires IsApartOf<U> void Set(U&& u) {
            if constexpr (std::is_same_v<T, std::remove_cvref_t<U>>) { first = std::forward<U>(u); }
            else { rest.Set(std::forward<U>(u)); }
        }

        template <class U> requires IsApartOf<U>
        void Destroy() { first.~U(); }
    };

    template <class T>
    union Union<T> {
        T first;

        template <class U> static constexpr bool IsApartOf = std::is_same_v<T, U>;

        Union(const T& t) : first(t) {}
        Union(T&& t) : first(std::move(t)) {}
        ~Union() {}

        Union(const Union&) = delete;
        Union& operator=(const Union&) = delete;

        template <std::same_as<T> U> const U& As() const { return first; }
        template <std::same_as<T> U> U& As() { return first; }

        template <class U> requires IsApartOf<U> void Set(U&& u) {
            first = std::forward<U>(u);
        }

        template <class U> requires IsApartOf<U>
        void Destroy() { first.~U(); }
    };
}
