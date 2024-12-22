#pragma once
#include <utility>

#include "Memory.h"

namespace Quasi {
    template <class Res, class... Args>
    using FuncPtr = Res(*)(Args...);

    template <class Fn>
    struct FuncRef {};

    template <class Result, class... Args>
    struct FuncRef<Result(Args...)> {
        void* userPtr = nullptr;
        FuncPtr<Result, void*, Args&&...> functionPtr = nullptr;

        Result Invoke(Args&&... args) const { return functionPtr(userPtr, std::forward<Args>(args)...); }
        Result operator()(Args&&... args) const { return Invoke(std::forward<Args>(args)...); }

        FuncRef() = default;
        FuncRef(void* uptr, FuncPtr<Result, void*, Args&&...> fptr) : userPtr(uptr), functionPtr(fptr) {}
        FuncRef(FuncPtr<Result, Args&&...> fptr) : userPtr(fptr), functionPtr(+[] (void* f, Args&&... args) {
            return (*Memory::UpcastPtr<FuncPtr<Result, Args&&...>>(f))(std::forward<Args>(args)...);
        }) {}

        FuncRef(const FuncRef& f) = default;
        FuncRef(FuncRef&& f) noexcept = default;

        template <class Lamb>
        FuncRef(Lamb&& lamb)
            : userPtr(&lamb),
            functionPtr(+[] (void* functionObj, Args&&... args) {
                return (*Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj))(std::forward<Args>(args)...);
            })
        {}

        template <class Lamb>
        static FuncRef Recursive(Lamb&& lamb) {
            return {
                &lamb,
                [] (void* functionObj, Args&&... args) {
                    auto* f = Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj);
                    return (*f)(f, std::forward<Args>(args)...);
                }
            };
        }

        template <class OtherFn>
        FuncRef<OtherFn> Transmute() const {
            return Memory::Transmute<FuncRef<OtherFn>>(*this);
        }
    };

    template <class T> struct Vec;

    namespace Combinate {
        struct Identity {
            template <class T> T&& operator()(T&& in) { return std::forward<T>(in); }
        };

        template <class F>
        struct Y {
            F func;
            auto operator()(auto&&... args) { return func(*this, std::forward<decltype(args)>(args)...); }
        };

        template <class F>
        struct Negate { F func; bool operator()(auto&&... args) { return !func(std::forward<decltype(args)>(args)...); } };

        template <class T>
        struct Collect {
            Vec<T> elements;
            void operator()(const T& val) { elements.Push(val); }
            void operator()(T&& val) { elements.Push(std::move(val)); }
        };

        template <class T, class F>
        struct Bind {
            T first;
            const F& func;
            auto operator()(auto&&... rest) const { return func(first, (decltype(rest))rest...); }
        };

        template <class T>
        struct Constructor {
            T operator()(auto&&... args) const { return T { (decltype(args))args... }; }
        };
    }

    namespace Operators {
        template <auto Indirect>
        struct Member      { auto& operator()(const auto& x) const { return x.*Indirect; } };
        template <auto Indirect>
        struct MemberArrow { auto& operator()(const auto& x) const { return x->*Indirect; } };
    }

    template <class F, class O, class... I>
    concept Fn = requires (F& f, I&&... args) {
        { f(std::forward<I>(args)...) } -> ConvTo<O>;
    };

    template <class F, class... I>
    concept FnArgs = requires (F& f, I&&... args) {
        { f(std::forward<I>(args)...) } -> AlwaysTrue;
    };

    template <class Fn, class... Ts> using FuncResult = decltype(std::declval<Fn>()(std::declval<Ts>()...));

    template <class Fn> struct ArgTypes {};
    template <class Res, class... Args> struct ArgTypes<Res(*)(Args...)> { using type = Tuple<Args...>; };
    template <class Fn> using ArgumentsOf = typename ArgTypes<Fn>::type;

    template <class F, class T>
    concept Predicate = Fn<F, bool, const T&>;
} // Q
