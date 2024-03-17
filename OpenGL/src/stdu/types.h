#pragma once
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include "NumTypes.h"

namespace stdu {
    struct empty {
        empty(auto ...) {}
    }; // used with [[no_unique_address]]

    template <class>
    concept always_true = true;

    template <class>
    concept always_false = false;

    template <class T> using is_void_t = std::is_same<T, void>;

    template <class T> constexpr bool is_void_v = std::is_same_v<T, void>;
    
    template <class T, class M> M decltype_member(M T::*) { return M {}; }

#define STDU_DECLTYPE_MEMBER(T, M) decltype(stdu::decltype_member(&T::M))

    template <class T, class U>
    struct arithmetic_t {
        static constexpr bool value = std::is_arithmetic_v<T> && std::is_arithmetic_v<U>;

        static T clamped_neg(T x) {
            if constexpr (std::is_unsigned_v<T>) return 0;
            else return -x;
        }

        static bool lt(T a, U b) { return a  < b; }
        static bool le(T a, U b) { return a <= b; }
        static bool eq(T a, U b) { return a == b; }
        static bool ge(T a, U b) { return a >= b; }
        static bool gt(T a, U b) { return a  > b; }
        static bool ne(T a, U b) { return a != b; }
    };

    template <class T, class U>
    concept convertible_to = std::is_convertible_v<T, U>;

    template <class R>
    concept array_like = std::ranges::contiguous_range<R> && std::ranges::sized_range<R>;

    template <class R>
    using element_t = std::ranges::range_value_t<R>;

    template <class R, class E>
    concept range_of = std::ranges::range<R> && std::is_same_v<std::ranges::range_value_t<R>, E>;

    template <array_like R> // from https://stackoverflow.com/questions/77097857/conversion-of-stdvector-to-stdspant
    std::span<element_t<R>> to_span(R& r) { return std::span<std::ranges::range_value_t<R>>(r); }

    template <array_like R>
    std::span<const element_t<R>> to_cspan(const R& r) { return std::span<const std::ranges::range_value_t<R>>(r); }

    template <class T, class U>
    concept divides = sizeof(U) % sizeof(T) == 0;

    template <class U, class T> requires divides<T, U> || divides<U, T>
    std::span<U> span_cast(std::span<T> span) { return { (U*)span.data(), span.size_bytes() / sizeof(U) }; }

    using byte_span = std::span<byte>;
    using cbyte_span = std::span<const byte>;

    // adapted from https://stackoverflow.com/a/66422428
    template <bool... Bs>
    constexpr bool all_of = (Bs && ...);

    template <bool... Bs>
    constexpr bool any_of = (Bs || ...);

    template <auto V> struct value_t { using type = decltype(V); static constexpr type value = V; };

    template <class, template <class> class> struct instanceof_t : std::false_type {};
    template <class T, template <class> class I> struct instanceof_t<I<T>, I> : std::true_type {};
    template <class T, template <class> class I>
    concept instanceof = instanceof_t<T, I>::value;

    template <class... Ts> struct typelist {
        static constexpr std::size_t size = sizeof...(Ts);
        template <class X> using join = typelist<Ts..., X>;
    };
    template <class> struct typelist_instance_t : std::false_type { };
    template <class... T> struct typelist_instance_t<typelist<T...>> : std::true_type { };
    template <class T> concept typelist_instance = typelist_instance_t<T>::value;

    template <class T, T ...V> struct valuelist {
        using type = T;
        static constexpr std::size_t size = sizeof...(V);
        template <T X> using join = valuelist<T, V..., X>;
    };
    template <class> struct valuelist_instance_t : std::false_type { };
    template <class T, T ...V> struct valuelist_instance_t<valuelist<T, V...>> : std::true_type { };
    template <class T> concept valuelist_instance = valuelist_instance_t<T>::value;

    template <auto ...V> struct anylist {
        using type = typelist<decltype(V)...>;
        static constexpr std::size_t size = sizeof...(V);
        template <auto X> using join = anylist<V..., X>;
    };
    template <class> struct anylist_instance_t : std::false_type { };
    template <auto ...V> struct anylist_instance_t<anylist<V...>> : std::true_type { };
    template <class T> concept anylist_instance = anylist_instance_t<T>::value;

    template <typelist_instance K, valuelist_instance V> requires (K::size == V::size)
    struct typemap { using value_type = typename V::type; };

    template <class U, class T>
    constexpr bool is_in = false;

    template <class U, class... Ts>
    constexpr bool is_in<U, typelist<Ts...>> = (std::is_same_v<U, Ts> || ...);

    template <class I, class T>
    constexpr int index_of_t = -1;

    template <class I, class... T>
    constexpr int index_of_t<I, typelist<I, T...>> = 0;

    template <class I, class U, class... Ts>
    constexpr int index_of_t<I, typelist<U, Ts...>> = 1 + index_of_t<I, typelist<Ts...>>;

    template <auto I, class T>
    constexpr int index_of_v = -1;

    template <class V, V I, V... T>
    constexpr int index_of_v<I, valuelist<V, I, T...>> = 0;

    template <class V, V I, V J, V... T>
    constexpr int index_of_v<I, valuelist<V, J, T...>> = 1 + index_of_v<I, valuelist<V, T...>>;

    template <auto I, auto... V>
    constexpr int index_of_v<I, anylist<I, V...>> = 0;

    template <auto I, auto J, auto... V>
    constexpr int index_of_v<I, anylist<J, V...>> = 1 + index_of_v<I, anylist<V...>>;

    template <class Q, class M>
    constexpr typename M::value_type query_map = {};

    template <class Q, class ...Ts, class V, V ...Vs>
    constexpr V query_map<Q, typemap<typelist<Ts...>, valuelist<V, Vs...>>>
        = std::get<index_of_t<Q, typelist<Ts...>>>(std::make_tuple(Vs...));

    template <auto Q, class M>
    struct reverse_query_map_t { using type = void; };

    template <class V, V Q, class... Ts, V... Vs>
    struct reverse_query_map_t<Q, typemap<typelist<Ts...>, valuelist<V, Vs...>>> {
        using type = decltype(std::get<index_of_v<Q, valuelist<V, Vs...>>>(
            std::make_tuple(std::declval<Ts>()...)));
    };

    template <auto Q, class M>
    using reverse_query_map = typename reverse_query_map_t<Q, M>::type;

    // adapted from https://stackoverflow.com/a/59522794/19968422
    namespace impl {
        template <class T>
        [[nodiscard]] constexpr std::string_view raw_typename() {
#if defined(__clang__) || defined(__GNUC__)
            return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
            return __FUNCSIG__;  // NOLINT(clang-diagnostic-language-extension-token)
#else
#error "Pretty Function Not supported"
#endif
        }

        constexpr std::size_t type_junk_prefix = raw_typename<int>().find("int");
        constexpr std::size_t type_junk_size = raw_typename<int>().size() - 3;
        // ReSharper disable once CppStaticAssertFailure
        static_assert(type_junk_prefix != std::string_view::npos, "cannot determine type signature in this compiler");
    }

    template <typename T>
    [[nodiscard]] constexpr std::string_view nameof() {
        const std::string_view fullname = impl::raw_typename<T>();
        return fullname.substr(impl::type_junk_prefix, fullname.size() - impl::type_junk_size);
    }

    template <typename T>
    struct is_complete_helper {
        template <typename U>
        static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)> { return {}; }
        static auto test(...) -> std::false_type { return {}; }
        using type = decltype(test((T*)nullptr));
    };

    template <typename T>
    struct is_complete : is_complete_helper<T>::type {};
    template <class T> constexpr bool is_complete_v = is_complete<T>::value;

    template <class F, class O, class... I>
    concept fn = requires (F f, I&&... args) {
        { f(std::forward<I>(args)...) } -> std::same_as<O>;
    };

    template <class F, class... I>
    concept fn_args = requires (F f, I&&... args) {
        { f(std::forward<I>(args)...) } -> always_true;
    };
}
