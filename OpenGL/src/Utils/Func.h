#pragma once
#include <bit>
#include <utility>

#include "Type.h"

namespace Quasi {
    template <class Res, class... Args>
    using FnPtr = Res(*)(Args...);

    template <class Fn>
    struct Func {};

    template <class Result, class... Args>
    struct Func<Result(Args...)> {
        void* userPtr = nullptr;
        FnPtr<Result, void*, Args&&...> functionPtr = nullptr;

        [[nodiscard]] Result Invoke(Args&&... args) const { return functionPtr(userPtr, std::forward<Args>(args)...); }
        [[nodiscard]] Result operator()(Args&&... args) const { return Invoke(std::forward<Args>(args)...); }

        Func() = default;
        Func(void* uptr, FnPtr<Result, void*, Args&&...> fptr) : userPtr(uptr), functionPtr(fptr) {}

        Func(const Func& f) = default;
        Func(Func&& f) noexcept = default;

        template <class Lamb>
        Func(Lamb&& lamb)
            : userPtr(&lamb),
            functionPtr([] (void* functionObj, Args&&... args) {
                return (*static_cast<std::decay_t<Lamb>*>(functionObj))(std::forward<Args>(args)...);
            })
        {}

        template <class Lamb>
        static Func Recursive(Lamb&& lamb) {
            return {
                &lamb,
                [] (void* functionObj, Args&&... args) {
                    auto* f = static_cast<std::decay_t<Lamb>*>(functionObj);
                    return (*f)(f, std::forward<Args>(args)...);
                }
            };
        }

        template <class OtherFn>
        Func<OtherFn> Transmute() const {
            return std::bit_cast<Func<OtherFn>>(*this);
        }
    };

    namespace Combinate {
        template <class F>
        struct Y {
            F func;
            auto operator()(auto&&... args) { return func(*this, std::forward<decltype(args)>(args)...); }
        };

        template <class T>
        struct Collect {
            Vec<T> elements;
            void operator()(const T& val) { elements.emplace_back(val); }
            void operator()(T&& val) { elements.emplace_back(std::move(val)); }
        };
    }

    template <class F, class O, class... I>
    concept Fn = requires (F f, I&&... args) {
        { f(std::forward<I>(args)...) } -> std::convertible_to<O>;
    };

    template <class F, class... I>
    concept FnArgs = requires (F f, I&&... args) {
        { f(std::forward<I>(args)...) } -> AlwaysTrue;
    };
} // Q
