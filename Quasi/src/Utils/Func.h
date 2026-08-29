#pragma once
#include <utility>

#include "Memory.h"
#include "Macros.h"

namespace Quasi {
    /// A raw C function pointer type.
    /// Identical to @code Res(*)(Args...) @endcode.
    /// @tparam Res The type of the function result.
    /// @tparam Args... The types of the function input parameters/arguments.
    template <class Res, class... Args>
    using FnPtr = Res(*)(Args...);

    template <class F>
    struct FnRef {};

    /// Represents a non-owning function reference which includes captures/closures.
    /// A general-purpose function reference wrapper that uses type erasure
    /// and support lambdas, raw function pointers or any function-like object that can be invoked.
    /// @note Internally, this is stored as two pointers:
    /// one that points to the closure/captured variables,
    /// and the other that points to the function itself.
    /// @tparam Result The type of the function result.
    /// @tparam Args... The types of the function input parameters/arguments.
    template <class Result, class... Args>
    struct FnRef<Result(Args...)> {
    private:
        void* userPtr = nullptr;
        FnPtr<Result, void*, Args...> functionPtr = &Empty;

        FnRef(void* uptr, FnPtr<Result, void*, Args...> fptr) : userPtr(uptr), functionPtr(fptr) {}
    public:
        /// The trivial function of doing nothing but the default. Essentially a no-op.
        static Result Empty(void*, Args...) { return Result {}; }
        /// Invokes the function.
        Result operator()(Args... args) const { return functionPtr(userPtr, (Args&&)args...); }

        FnRef() = default;
        /// Constructs the trivial 'do nothing' function reference.
        FnRef(Nullptr) : FnRef() {}
        /// Constructs a reference to a C function pointer.
        FnRef(FnPtr<Result, Args...> fptr) : userPtr(fptr), functionPtr(+[] (void* f, Args... args) {
            return (*Memory::UpcastPtr<FnPtr<Result, Args...>>(f))((Args&&)args...);
        }) {}

        /// Creates a function reference by its raw parts: the closure pointer and the function accepting the closure.
        static FnRef FromRaw(void* uptr, FnPtr<Result, void*, Args...> fptr) { return { uptr, fptr }; }

        /// Constructs a reference to the lambda/function-like object @p lamb.
        template <class Lamb> requires DistantTo<Lamb, FnRef>
        FnRef(Lamb&& lamb)
            : userPtr(&lamb),
            functionPtr(+[] (void* functionObj, Args... args) {
                return (*Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj))((Args&&)args...);
            })
        {}

        /// Checks if the function is in a valid and callable state.
        /// Should never be @p false unless uninitialized.
        operator bool() const { return functionPtr; }

        /// Constructs a reference to a lambda/function-like object @p lamb
        /// that shall be recursed when called.
        ///
        /// More specifically, calling this newly constructed reference
        /// passes the lambda itself as the first argument to support recursion.
        ///
        /// @b Example: @code
        /// const auto fib = FuncRef<int, int>::Recursive([&] (auto& self, int n) {
        ///     if (n == 0) return 0;
        ///     if (n == 1) return 1;
        ///     return self(n - 1) + self(n - 2);
        /// });
        /// @endcode
        template <class Lamb>
        static FnRef Recursive(Lamb&& lamb) {
            return {
                &lamb,
                [] (void* functionObj, Args... args) {
                    auto* f = Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj);
                    return (*f)(f, (Args&&)args...);
                }
            };
        }

        /// Reinterpret casts the current function @code Result(*)(Args...)@endcode
        /// to the function type @p OtherFn.
        /// May be used if one is @b certain that the argument types are supposed
        /// to be interchangable between function types.
        /// @warning Should this be cast to function type with an
        /// incompatible/mismatching calling convention, the behavior of this UB.
        /// @tparam OtherFn The function type to reinterpret/transmute to.
        template <class OtherFn>
        FnRef<OtherFn> Transmute() const {
            return Memory::Transmute<FnRef<OtherFn>>(*this);
        }
    };

    /// The trivial destructor that does nothing.
    inline void ZeroDestructor(void*) {}

    template <class F>
    struct FnBox {};

    /// Represents a owning function object which includes captures/closures.
    /// A general-purpose function wrapper with RAII that uses type erasure
    /// and support lambdas, raw function pointers or any function-like object that can be invoked.
    /// Memory is managed by the object internally. Does not support copying.
    /// @note Internally, this is stored as three pointers:
    /// - a pointer to the closure/captured variables,
    /// - the destructor function pointer for the closure,
    /// - and one that points to the function itself.
    /// @tparam Result The type of the function result.
    /// @tparam Args... The types of the function input parameters/arguments.
    template <class Result, class... Args>
    struct FnBox<Result(Args...)> {
    private:
        void* userPtr = nullptr;
        FnPtr<void, void*> destructor = &ZeroDestructor;
        FnPtr<Result, void*, Args...> functionPtr = &FnRef<Result(Args...)>::Empty;

        FnBox(void* uptr, FnPtr<Result, void*, Args...> fptr) : userPtr(uptr), functionPtr(fptr) {}
        FnBox(void* uptr, FnPtr<void, void*> destructor, FnPtr<Result, void*, Args...> fptr)
            : userPtr(uptr), destructor(destructor), functionPtr(fptr) {}
    public:
        /// Invokes the function.
        Result operator()(Args... args) const { return functionPtr(userPtr, (Args&&)args...); }

        FnBox() = default;
        /// Constructs the trivial 'do nothing' function reference.
        FnBox(Nullptr) : FnBox() {}
        /// Constructs a reference to a C function pointer.
        FnBox(FnPtr<Result, Args...> fptr) : userPtr(fptr), functionPtr(+[] (void* f, Args... args) {
            return (*Memory::UpcastPtr<FnPtr<Result, Args...>>(f))((Args&&)args...);
        }) {}
        ~FnBox() {
            if (userPtr)
                destructor(userPtr);
        }

        /// Creates an owned function by its raw parts:
        /// the closure pointer,
        /// the function accepting the closure,
        /// and the destructor for the closure.
        static FnBox FromRaw(
            void* uptr,
            FnPtr<Result, void*, Args...> fptr,
            FnPtr<void, void*> dtor = &ZeroDestructor) { return { uptr, dtor, fptr }; }

        FnBox(const FnBox& f) = delete;
        FnBox(FnBox&& f) noexcept {
            userPtr     = f.userPtr;     f.userPtr     = nullptr;
            destructor  = f.destructor;  f.destructor  = nullptr;
            functionPtr = f.functionPtr; f.functionPtr = nullptr;
        }
        FnBox& operator=(const FnBox& f) = delete;
        FnBox& operator=(FnBox&& f) noexcept {
            if (userPtr) destructor(userPtr);
            userPtr     = f.userPtr;     f.userPtr     = nullptr;
            destructor  = f.destructor;  f.destructor  = nullptr;
            functionPtr = f.functionPtr; f.functionPtr = nullptr;
            return *this;
        }

        /// Constructs an owned function to the lambda/function-like object @p lamb.
        template <class Lamb> requires DistantTo<Lamb, FnBox>
        FnBox(Lamb&& lamb)
            : userPtr(new Lamb(std::forward<Lamb>(lamb))), destructor([] (void* f) { delete (Lamb*)f; }),
            functionPtr(+[] (void* functionObj, Args... args) {
                return (*Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj))((Args&&)args...);
            })
        {}

        /// Checks if the function is in a valid and callable state.
        /// Should never be @p false unless uninitialized.
        operator bool() const { return functionPtr; }

        /// Constructs an owned function to a lambda/function-like object @p lamb
        /// that shall be recursed when called.
        ///
        /// More specifically, calling this newly constructed object
        /// passes the lambda itself as the first argument to support recursion.
        ///
        /// @b Example: @code
        /// const auto fib = FuncRef<int, int>::Recursive([&] (auto& self, int n) {
        ///     if (n == 0) return 0;
        ///     if (n == 1) return 1;
        ///     return self(n - 1) + self(n - 2);
        /// });
        /// @endcode
        template <class Lamb>
        static FnBox Recursive(Lamb&& lamb) {
            return {
                &lamb,
                [] (void* functionObj, Args... args) {
                    auto* f = Memory::UpcastPtr<std::decay_t<Lamb>>(functionObj);
                    return (*f)(f, (Args&&)args...);
                }
            };
        }

        /// Reinterpret casts the current function @code Result(*)(Args...)@endcode
        /// to a reference to a function type @p OtherFn.
        /// May be used if one is @b certain that the argument types are supposed
        /// to be interchangable between function types.
        /// @warning Should this be cast to function type with an
        /// incompatible/mismatching calling convention, the behavior of this UB.
        /// @tparam OtherFn The function type to reinterpret/transmute to.
        template <class OtherFn>
        FnRef<OtherFn> Transmute() const {
            return AsRef().template Transmute<OtherFn>();
        }

        /// Returns a non-owning reference to the owning function object.
        FnRef<Result(Args...)> AsRef() const {
            return FnRef<Result(Args...)>::FromRaw(userPtr, functionPtr);
        }
    };

    namespace FnRefs {
        template <class O, class... Is>
        FnRef<O(Is...)> FromRaw(void* user, O(*fptr)(void*, Is...)) { return FnRef<O(Is...)>::FromRaw(user, fptr); }
    }

    template <class T> struct Vec;

    /// The Identity function. Anything that is passed in is passed out (forwarded).
    struct Identity {
        template <class T> T&& operator()(T&& in) { return std::forward<T>(in); }
    };

    /// Provides useful functions/higher-order-functions that help achieve effective functional programming practices.
    namespace Fns {
        /// The Y-combinator. Allows for functions to be recusrive by letting them access themselves.
        ///
        /// More specifically, calling this object passes the underlying function
        /// itself as the first argument to support recursion.
        /// @b Example: @code
        /// const auto fib = Fns::Y([&] (auto& self, int n) {
        ///     if (n == 0) return 0;
        ///     if (n == 1) return 1;
        ///     return self(n - 1) + self(n - 2);
        /// });
        /// @endcode
        template <class F>
        struct Y {
            F func;
            auto operator()(auto&&... args) { return func(*this, std::forward<decltype(args)>(args)...); }
        };

        /// The collector function. Everytime this function is called with a value,
        /// it is collected into a @code Vec<T>@endcode, which can then be extracted later.
        /// @tparam T The element type to collect.
        template <class T>
        struct Collect {
            Vec<T> elements;
            void operator()(const T& val) { elements.Push(val); }
            void operator()(T&& val) { elements.Push(std::move(val)); }
        };

        /// Adds indexing to a pre-existing function,
        /// which counts how many times the function has been called already.
        /// The index/number of times the function is called is provided as the first argument.
        ///
        /// @b Example: @code
        /// const auto f = Fns::Counting([] (usize i, float x) {
        ///     Text::Print("{}, ", i);
        ///     return x * i;
        /// });
        /// f(10); // returns 10
        /// f(25); // returns 50
        /// f(50); // returns 75
        /// // prints: 1, 2, 3,
        /// @endcode
        template <class F>
        struct Counting : F {
            F func;
            usize i = 0;
            auto operator()(auto&&... args) { return func(i++, (decltype(args))args...); }
        };

        /// The constructor function for @p T,
        /// which forwards the input parameters to the constructor for @p T
        /// and returns the newly created object.
        template <class T>
        struct Constructor {
            T operator()(auto&&... args) const { return T { (decltype(args))args... }; }
        };

        /// A function object that supports multiple overloads and dispatches them at compile time.
        /// Useful for applying the visitor pattern.
        ///
        /// @b Example: @code
        /// const auto f = Fns::Overload(
        ///     [] (int x)   { Text::PrintLn("got the int {}!", x); }
        ///     [] (float x) { Text::PrintLn("got the float {}!", x); }
        /// );
        /// f(12);
        /// f(3.4f);
        /// // prints:
        /// // got the int 12!
        /// // got the float 3.4!
        /// @endcode
        template <class... Fs>
        struct Overload : Fs... {
            using Fs::operator()...;
        };
        template <class... Fs>
        Overload(Fs&&...) -> Overload<Fs...>;


        template <class...> struct Compose;

        /// Composes/Chains a list of functions together, aka the composition operator.
        /// Identical to calling @code f(g(h(...))) @endcode
        /// when the object @code Compose(f, g, h)@endcode is invoked.
        /// @b Example: @code
        /// const auto f = Fns::Compose(
        ///     [] (float x) { return 1.0f / x; }
        ///     std::sqrt
        /// );
        /// f(0.25f); // performs sqrt(1/x) -> returns 4
        /// @endcode
        template <class F, class... Fs>
        struct Compose<F, Fs...> {
            F first;
            Compose<Fs...> rest;
            Compose(F first, Fs... rest) : first(std::move(first)), rest(std::move(rest)...) {}
            auto operator()(auto&&... xs) {
                return first(rest((decltype(xs))xs...));
            }
        };
        template <class F>
        struct Compose<F> : F {
            using F::operator();
        };
        template <class... Fs>
        Compose(Fs&&...) -> Compose<Fs...>;
    }

    namespace Operators {
        /// The member access operator. (x.*member)
        /// @tparam M The address to a member variable.
        template <auto M> struct Member { auto& operator()(auto&& x) const { return x.*M; } };
        /// The member arrow access operator. (x->*member)
        /// @tparam M The address to a member variable.
        template <auto M> struct MemberArrow { auto& operator()(auto&& x) const { return x->*M; } };

        // The destructor function of an object.
        struct Destructor {
            template <class T>
            void operator()(T& t) const { t.~T(); }
        };

#define Q_DEF_UNARY_OPERATOR_FUNCTOR(NAME, OP) struct NAME { auto operator()(auto&& x) const { return OP; } };
#define Q_DEF_OPERATOR_FUNCTOR(NAME, OP) struct NAME { auto operator()(auto&& x, auto&& y) const { return OP; } };
        /* The unary plus operator.     (+x) */     Q_DEF_UNARY_OPERATOR_FUNCTOR(UPos, +x)
        /* The unary minus operator.    (-x) */     Q_DEF_UNARY_OPERATOR_FUNCTOR(UNeg, -x)
        /* The addition operator.       (x + y) */  Q_DEF_OPERATOR_FUNCTOR(Add, x + y)
        /* The subtraction operator.    (x - y) */  Q_DEF_OPERATOR_FUNCTOR(Sub, x - y)
        /* The multiplication operator. (x * y) */  Q_DEF_OPERATOR_FUNCTOR(Mul, x * y)
        /* The division operator.       (x / y) */  Q_DEF_OPERATOR_FUNCTOR(Div, x / y)
        /* The modulo operator.         (x % y) */  Q_DEF_OPERATOR_FUNCTOR(Mod, x % y)
        /* The add-assignment operator. (x += y) */ Q_DEF_OPERATOR_FUNCTOR(AddAssign, x += y)
        /* The sub-assignment operator. (x -= y) */ Q_DEF_OPERATOR_FUNCTOR(SubAssign, x -= y)
        /* The mul-assignment operator. (x *= y) */ Q_DEF_OPERATOR_FUNCTOR(MulAssign, x *= y)
        /* The div-assignment operator. (x /= y) */ Q_DEF_OPERATOR_FUNCTOR(DivAssign, x /= y)
        /* The mod-assignment operator. (x %= y) */ Q_DEF_OPERATOR_FUNCTOR(ModAssign, x %= y)

        /* The increment operator. (++x) */      Q_DEF_UNARY_OPERATOR_FUNCTOR(Inc, ++x)
        /* The decrement operator. (--x) */      Q_DEF_UNARY_OPERATOR_FUNCTOR(Dec, --x)
        /* The post-increment operator. (x++) */ Q_DEF_UNARY_OPERATOR_FUNCTOR(PostInc, x++)
        /* The post-decrement operator. (x--) */ Q_DEF_UNARY_OPERATOR_FUNCTOR(PostDec, x--)

        /* 2's complement operator (~x) */                      Q_DEF_UNARY_OPERATOR_FUNCTOR(Compl, ~x)
        /* Bitwise AND operator         (x & y) */              Q_DEF_OPERATOR_FUNCTOR(BitAnd, x &  y)
        /* Bitwise OR operator          (x | y) */              Q_DEF_OPERATOR_FUNCTOR(BitOr,  x |  y)
        /* Bitwise XOR operator         (x ^ y) */              Q_DEF_OPERATOR_FUNCTOR(BitXor, x ^  y)
        /* Bitwise left shift operator  (x << y) */             Q_DEF_OPERATOR_FUNCTOR(BitShl, x << y)
        /* Bitwise right shift operator (x >> y) */             Q_DEF_OPERATOR_FUNCTOR(BitShr, x >> y)
        /* Bitwise AND assignment operator         (x &= y) */  Q_DEF_OPERATOR_FUNCTOR(BitAndAssign, x &=  y)
        /* Bitwise OR assignment operator          (x |= y) */  Q_DEF_OPERATOR_FUNCTOR(BitOrAssign,  x |=  y)
        /* Bitwise XOR assignment operator         (x ^= y) */  Q_DEF_OPERATOR_FUNCTOR(BitXorAssign, x ^=  y)
        /* Bitwise left shift assignment operator  (x <<= y) */ Q_DEF_OPERATOR_FUNCTOR(BitShlAssign, x <<= y)
        /* Bitwise right shift assignment operator (x >>= y) */ Q_DEF_OPERATOR_FUNCTOR(BitShrAssign, x >>= y)

        /* The boolean NOT operator (!x) */     Q_DEF_UNARY_OPERATOR_FUNCTOR(Not, !x)
        /* The boolean AND operator (x && y) */ Q_DEF_OPERATOR_FUNCTOR(And, x && y)
        /* The boolean OR operator  (x && y) */ Q_DEF_OPERATOR_FUNCTOR(Or,  x || y)

        /* The array index operator (x[y]) */ Q_DEF_OPERATOR_FUNCTOR(Index, x[y])

#undef Q_DEF_OPERATOR_FUNCTOR
#undef Q_DEF_UNARY_OPERATOR_FUNCTOR

        /// Takes the proper modulo of two values.
        /// The difference between @p Modulo and @p NumericModulo
        /// is that @p NumericModulo calls @p std::fmod for floating point values.
        struct NumericModulo { template <class T> T operator()(T x, T y) const { return NumInfo<T>::Modulo(x, y); } };
    }

    /// Checks if a function will have return type @p O when provided with inputs of types @p I .
    template <class F, class O, class... I>
    concept Fn = requires (F& f, I&&... args) {
        { f(std::forward<I>(args)...) } -> ConvTo<O>;
    };

    /// Checks if a function can be called with inputs of types @p I, with no constraints in the output type.
    template <class F, class... I>
    concept FnArgs = requires (F& f, I&&... args) {
        { f(std::forward<I>(args)...) } -> AlwaysTrue;
    };

    /// Yields the type of the function result which is called by using @p Ts as input arguments.
    ///
    /// @b Example: @code
    /// FnResult<Operators::Mul, int, int>     // -> int
    /// FnResult<Operators::Index, char*, int> // -> char
    /// FnResult<Operators::Add, float, int>   // -> float
    /// @endcode
    template <class F, class... Ts> using FnResult = decltype(std::declval<F>()(std::declval<Ts>()...));

    /// Checks whether the input type is a @b predicate;
    /// i.e. a function that takes in a value by const reference and returns either @p true or @p false.
    ///
    /// Identical to @code Fn<_, bool, const T&>@endcode.
    template <class F, class T>
    concept Predicate = Fn<F, bool, const T&>;

#define Q_LMB_TAKES_ARGS(...) , Q_TINY_LAMBDA_SIMPLE,
#define Q_TINY_LAMBDA_ARGS(...) [&] (__VA_ARGS__) { return
#define Q_TINY_LAMBDA_SIMPLE(...) Q_TINY_LAMBDA_ARGS __VA_ARGS__ ; }
#define Q_TINY_LAMBDA_FORWARD_TO(M) [&] (auto&&... args) { return M((decltype(args))args...); }
#define Q_TINY_LAMBDA(...) Q_INVOKE(Q_ARGS_SECOND, Q_LMB_TAKES_ARGS __VA_ARGS__, Q_TINY_LAMBDA_FORWARD_TO) (__VA_ARGS__)

    /// Syntax sugar for writing simple lambda expressions.
    ///
    /// Supports two forms:
    /// - @p Qfn$(FUNC) turns the pre-existing function @p FUNC into a lambda by forwarding its arguments
    /// - @code Qfn$((T x) x + 1)@endcode is equivalent to writing @code [&] (T x) { return x + 1; }@endcode.
    /// Supports multiple input parameters.
#define Qfn$(...) Q_TINY_LAMBDA(__VA_ARGS__)
}
