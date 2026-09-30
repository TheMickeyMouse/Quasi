#pragma once
#include <cmath>

#include "Utils/Text/Num.h"

#include "Utils/Array.h"
#include "Utils/Iter/Split.h"
#include "Utils/Iter/Enumerate.h"

namespace Quasi::Math {
    struct Rotor2D;
    struct RandomGenerator;
    struct Transform2D;
    template <class T> struct Complex;

    template <class T, usize N> struct Vector;
    using iv2 = Vector<int,    2>;
    using iv3 = Vector<int,    3>;
    using iv4 = Vector<int,    4>;
    using uv2 = Vector<uint,   2>;
    using uv3 = Vector<uint,   3>;
    using uv4 = Vector<uint,   4>;
    using fv2 = Vector<float,  2>;
    using fv3 = Vector<float,  3>;
    using fv4 = Vector<float,  4>;
    using dv2 = Vector<double, 2>;
    using dv3 = Vector<double, 3>;
    using dv4 = Vector<double, 4>;
    using bv2 = Vector<byte,   2>;
    using bv3 = Vector<byte,   3>;
    using bv4 = Vector<byte,   4>;
    template <class T> using Unit = Vector<T, 1>;
    template <class T> using Vec2 = Vector<T, 2>;
    template <class T> using Vec3 = Vector<T, 3>;
    template <class T> using Vec4 = Vector<T, 4>;

    template <class T, usize N> struct Rect;

    /// The base class for all vectorial types @code Vector<T, N>@endcode.
    /// Adds basic methods to reduce code repetition.
    ///
    /// To implement this class, subclasses must implement the following methods:
    /// @code
    /// T* DataImpl();
    /// const T* DataImpl() const;
    /// static usize CompFromNameImpl(char c);
    /// @endcode
    /// @tparam T The element/component type for the vector.
    /// @tparam N The number of dimensions of the vector.
    template <class T, usize N> struct IVector {
        using Super = Vector<T, N>;
        using Elm = T;
        enum { Dim = N };
        using fT = Common<T, f32>;
    private:
        const Super& super() const { return *static_cast<const Super*>(this); }
        Super& super() { return *static_cast<Super*>(this); }
    protected:
        T*       DataImpl() = delete;
        const T* DataImpl() const = delete;
        static usize CompFromNameImpl(char c) = delete;
    public:

        /// Gets the index of a component given its character name,
        /// i.e. @p 'x', @p 'y', @p 'z', @p 'w', or even one of @p 'rgba'/'stuv'
        static usize CompFromName(char c) { return Super::CompFromNameImpl(c); }
        /// Gets the pointer pointing to the elements of the vector, as if it were a C-array.
        T* Data() { return super().DataImpl(); }
        /// Gets the const pointer pointing to the elements of the vector, as if it were a C-array.
        const T* Data() const { return super().DataImpl(); }
        /// Gets the elements of the vector as a span.
        Span<T> AsSpan() { return Spans::Slice(Data(), N); }
        /// Gets the elements of the vector as a const span.
        Span<const T> AsSpan() const { return Spans::Slice(Data(), N); }

        /// Gets the @p i th element of the vector.
        T& At(usize i) { return Data()[i]; }
        /// Gets the @p i th element of the vector.
        const T& At(usize i) const { return Data()[i]; }
        /// Gets the @p i th element of the vector.
        T& operator[](usize i) { return Data()[i]; }
        /// Gets the @p i th element of the vector.
        const T& operator[](usize i) const { return Data()[i]; }

        /// Performs a vector swizzle operation, where the indices are specified by name (ex: xyzw, rgba, stuv)
        /// and the whole swizzle is represented by a string of characters @p swizzle .
        ///
        /// @b Example: @code
        /// iv3 v = { 1, 2, -3 };
        /// iv3 u = v["zxy"]; // .Swizzle("zxy") also works
        /// // u == { -3, 1, 2 }
        /// @endcode
        ///
        /// @warning UB if @p swizzle contains a character that @a isn't one of 'xyzw', 'rgba' or 'stuv';
        /// or the index is out of bounds.
        template <usize M>
        Vector<T, M - 1> Swizzle(const char (&swizzle)[M]) const {
            Vector<T, M - 1> swizzled;
            for (usize i = 0; i < M - 1; ++i) {
                const usize comp = CompFromName(swizzle[i]);
                swizzled[i] = comp == -1 ? 0 : At(comp);
            }
            return swizzled;
        }
        /// Performs a vector swizzle operation. See @p Swizzle .
        template <usize M> Vector<T, M - 1> operator[](const char (&swizzle)[M]) { return Swizzle(swizzle); }

        /// Returns the number of dimensions of the vector.
        /// Is a constant operation and can be access with @p VectorType::DIM as well.
        usize Dimensions() const { return N; }

        static Super FromSpan(Span<const T> elms) {
            Super v;
            Memory::RangeCopy(v.Data(), elms.Data(), std::min(elms.Length(), N));
            return v;
        }
        // static Super FromAxis(some axis type, T scale);
        static Super FromCorner(const bool (&rel) [N], T scale = 1) {
            Super v;
            for (usize i = 0; i < N; ++i) v[i] = rel[i] ? scale : -scale;
            return v;
        }
        static Super FromScale(T scale) {
            Super v;
            Memory::RangeSet(v.Data(), scale, N);
            return v;
        }
        static Super Zero() { return {}; }
        static Super One() { return FromScale(1); }

        /// Returns a new vector with the element @p extra tagged along at the back, similar to @p Vec<T>::Push .
        Vector<T, N + 1> AddComponent(T extra) const {
            Vector<T, N + 1> v;
            Memory::RangeCopy(v.Data(), Data(), N);
            v[N] = extra;
            return v;
        }
        /// Returns a new vector with <em>one less</em> element at the back, similar to @p Vec<T>::Pop .
        Vector<T, N - 1> RemoveComponent() const {
            Vector<T, N - 1> v;
            Memory::RangeCopy(v.Data(), Data(), N - 1);
            return v;
        }

        /// Casts the vector into a different one of element type @p U and of dimension @p M .
        /// If the new vector type is longer (has higher dimensions) than the current,
        /// then the missing values are filled with zeros. Otherwise, the elements are truncated instead.
        template <class U, usize M = N> Vector<U, M> As() const {
            Vector<U, M> v;
            static constexpr usize CopyLen = std::min(N, M);
            for (usize i = 0; i < CopyLen; ++i) v[i] = (U)At(i);
            return v;
        }

        /// Casts the vector into a different vector type. See @p As .
        template <class U, usize M> explicit operator Vector<U, M>() const { return As<U, M>(); }

        /// Checks whether the vector is @a exactly the zero vector.
        /// @note For floating point vector types, one may want to use the more
        /// lenient, although slower, @p NearZero instead.
        bool IsZero() const { return super() == 0; }
        /// Checks whether the vector has a length of less than 10⁻⁷ (@p FLT_EPSILON).
        /// @note This may be prefered over @p IsZero for floating point vectors if calculation precision is an issue.
        bool NearZero() const { return LenSq() < (T)(f32s::DELTA * f32s::DELTA); }
        /// Defines a total ordering for vectors. Useful for sorting or hashing.
        /// @note Uses the ordering defined by @p Span internally.
        Comparison Cmp(const Super& other) const { return AsSpan().Cmp(other.AsSpan()); }

        /// The unary plus operator. Does nothing.
        Super operator+() const { return super(); }
        /// The unary minus operator. Negates the vector.
        Super operator-() const { return Map(Operators::UNeg {}); }

        /// Adds two vectors together.
        Super operator+(const Super& v) const { return BinaryMap(v, Operators::Add {}); }
        /// Adds the value @p x to each component in the vector.
        Super operator+(T x) const { return Map(x, Operators::Add {}); }
        /// Subtracts two vectors together.
        Super operator-(const Super& v) const { return BinaryMap(v, Operators::Sub {}); }
        /// Subtracts the value @p x to each component in the vector.
        Super operator-(T x) const { return Map(x, Operators::Sub {}); }
        /// Performs component-wise multiplication; i.e., @code a * b == { a[0] * b[0], a[1] * b[1], ... }@endcode.
        /// @note For the dot product, see @p Dot ; and for the cross product, see @p Cross .
        Super operator*(const Super& v) const { return BinaryMap(v, Operators::Mul {}); }
        /// Multiplies the vector by some scaling factor @p x .
        Super operator*(T x) const { return Map(x, Operators::Mul {}); }
        /// Performs component-wise division; i.e., @code a / b == { a[0] / b[0], a[1] / b[1], ... }@endcode.
        Super operator/(const Super& v) const { return BinaryMap(v, Operators::Div {}); }
        /// Divides the vector by some shrinking factor @p x .
        Super operator/(T x) const { return Map(x, Operators::Div {}); }
        /// Performs component-wise modulo; i.e., @code a % b == { a[0] % b[0], a[1] % b[1], ... }@endcode.
        Super operator%(const Super& v) const { return BinaryMap(v, Operators::NumericModulo {}); }
        /// Takes the modulo of each component with respect to @p x ; i.e., @code a % x == { a[0] % x, a[1] % x, ... }@endcode.
        Super operator%(T x) const { return Map(x, Operators::NumericModulo {}); }

        /// Adds @p v to this vector in place.
        Super& operator+=(const Super& v) { return BinaryAssign(v, Operators::AddAssign {}); }
        /// Adds the value @p x to each component in this vector in place.
        Super& operator+=(T x) { return MapAssign(x, Operators::AddAssign {}); }
        /// Subtracts @p v to this vector in place.
        Super& operator-=(const Super& v) { return BinaryAssign(v, Operators::SubAssign {}); }
        /// Subtracts the value @p x to each component in this vector in place.
        Super& operator-=(T x) { return MapAssign(x, Operators::SubAssign {}); }
        /// Performs component-wise multiplication onto this vector in place; see @p operator* .
        Super& operator*=(const Super& v) { return BinaryAssign(v, Operators::MulAssign {}); }
        /// Scales this vector by @p x in place.
        Super& operator*=(T x) { return MapAssign(x, Operators::MulAssign {}); }
        /// Performs component-wise division onto this vector in place; see @p operator/ .
        Super& operator/=(const Super& v) { return BinaryAssign(v, Operators::DivAssign {}); }
        /// Divides this vector by @p x in place.
        Super& operator/=(T x) { return MapAssign(x, Operators::DivAssign {}); }
        /// Performs component-wise modulo onto this vector in place; see @p operator% .
        Super& operator%=(const Super& v) { return super() = super() % v; }
        /// Takes the modulo by @p x on each component of this vector in place; see @p operator% .
        Super& operator%=(T x) { return super() = super() % x; }

        /// Returns a vector where each element has some function @p mapping applied to it,
        /// or in other words, @code { mapping(v[0]), mapping(v[1]), ... }@endcode (<em>internal use</em>)
        auto Map(const FnArgs<T> auto& mapping) const {
            Vector<FnResult<decltype(mapping), T>, N> v;
            for (usize i = 0; i < N; ++i) { v[i] = mapping(At(i)); }
            return v;
        }
        /// Returns a vector where each element has some function @p mapping applied to it,
        /// provided with an external argument @p other. In other words,
        /// @code { mapping(v[0], other), mapping(v[1], other), ... }@endcode (<em>internal use</em>)
        auto Map(T other, const FnArgs<T, T> auto& mapping) const {
            Vector<FnResult<decltype(mapping), T, T>, N> v;
            for (usize i = 0; i < N; ++i) { v[i] = mapping(At(i), other); }
            return v;
        }
        /// Returns a vector where each element is the result of some function @p mapping that operates
        /// on @a both of the elements from this vector and @p other. In other words,
        /// @code { mapping(v[0], other[0]), mapping(v[1], other[1]), ... }@endcode (<em>internal use</em>)
        auto BinaryMap(const Super& other, const FnArgs<T, T> auto& mapping) const {
            Vector<FnResult<decltype(mapping), T, T>, N> v;
            for (usize i = 0; i < N; ++i) { v[i] = mapping(At(i), other[i]); }
            return v;
        }
        /// Applies a function @p mapping onto each element <em>in place</em>, with an additional argument @p other provided.
        /// In other words, @code mapping(v[0], other); mapping(v[1], other); ...@endcode. (<em>internal use</em>)
        Super& MapAssign(T other, const FnArgs<T&, T> auto& mapping) {
            for (usize i = 0; i < N; ++i) mapping(At(i), other);
            return super();
        }
        /// Applies a function @p mapping that operates on @a both of the elements from this vector and @p other.
        /// In other words, @code mapping(v[0], other[0]); mapping(v[1], other[1]); ...@endcode. (<em>internal use</em>)
        Super& BinaryAssign(const Super& other, const FnArgs<T&, T> auto& mapping) {
            for (usize i = 0; i < N; ++i) mapping(At(i), other[i]);
            return super();
        }

        /// Returns the length of the vector, <b>‖v‖</b> (Euclidean distance).
        fT Len() const { return std::sqrt((fT)LenSq()); }
        /// Returns the squared length of the vector, <b>‖v‖²</b> (Euclidean distance).
        /// Useful for comparing distances since this does not require a sqrt.
        T  LenSq() const { return Dot(super()); }
        /// Returns the distance between two vectors, <b>‖a-b‖</b> (Euclidean distance).
        fT Dist(const Super& to) const { return (super() - to).Len(); }
        /// Returns the distance between two vectors, <b>‖a-b‖²</b> (Euclidean distance).
        T  DistSq(const Super& to) const { return (super() - to).LenSq(); }
        /// Checks whether the vector is less than or equal to a distance @p d away from the vector @p other .
        /// @note Does not require sqrt, compares using @p DistSq instead.
        bool InRange(const Super& other, T d) const { return DistSq(other) <= d * d; }
        /// Returns the normalized the vector, <b>v/‖v‖</b>.
        /// If a length argument is passed, scales the vector such that its length matches @p d: <b>d(v/‖v‖)</b>.
        Super Norm(T d = 1) const requires Floating<T> { return super() * ((T)d / Len()); }
        /// Returns both the normalized vector @a and the length.
        Tuple<Super, fT> NormAndLen() const requires Floating<T> { const fT len = Len(); return { super() * ((T)1 / len), len }; }
        /// Returns the tangent unit vector to the vector @p other: <b>(a-b)/‖a-b‖</b>.
        Super Tangent(const Super& other) const requires Floating<T> { return (other - super()).Norm(); }
        /// Gets the direction unit vector and distance to the vector @p other: <b>(a-b)/‖a-b‖ & ‖a-b‖</b>.
        Tuple<Super, fT> DirAndDist(const Super& other) const requires Floating<T> { return (other - super()).NormAndLen(); }
        /// Returns the normalized vector @a if the vector has a length greater than 10⁻⁷, otherwise returns the 0 vector instead.
        Super SafeNorm() const requires Floating<T> { return NearZero() ? 0 : Norm(); }

        /// Computes the dot product between two vectors, <b>a·b</b>
        T Dot(const Super& other) const {
            T sum = At(0) * other[0];
            for (usize i = 1; i < N; ++i) sum += At(i) * other[i];
            return sum;
        }
        /// Calculates the vector projection with respect to @p axis, or in other words,
        /// @a only the vector component parallel to it is returned: <b>a∥ = b(a·b)/‖b‖²</b>
        /// @note This is the complementary to @p Reject, the vector rejection. Their sum is always the original vector. @n @n
        /// If one is knows beforehand that @p axis is normalized, use @p ProjectNorm instead.
        Super Project(const Super& axis) const { return axis * (Dot(axis) / axis.LenSq()); }
        /// Calculates the vector rejection with respect to @p axis, or in other words,
        /// @a only the vector components perpendicular to it are returned: <b>a⟂ = a - b(a·b)/‖b‖²</b>
        /// @note This is the complementary to @p Project, the vector projection. Their sum is always the original vector. @n @n
        /// If one is knows beforehand that @p axis is normalized, use @p RejectNorm instead.
        Super Reject(const Super& axis) const { return super() - Project(axis); }
        /// Calculates the vector reflecection with respect to @p axis, or in other words,
        /// the vector component parallel to it is @a negated, while preserving all other directions:
        ///
        /// <b>proj_b(a) = a⟂ - a∥ = a - b(2a·b)/‖b‖²</b>
        /// @note Equal to @code Reject(axis) - Project(axis)@endcode. @n @n
        /// If one is knows beforehand that @p axis is normalized, use @p ReflectNorm instead.
        Super Reflect(const Super& axis) const { return super() - axis * (2 * Dot(axis) / axis.LenSq()); }

        /// Calculates the vector projection with respect to the @b normalized vector @p normAxis, or in other words,
        /// @a only the vector component parallel to it is returned: <b>a∥ = b̂(a·b̂)</b>
        /// @note This is the complementary to @p RejectNorm, the vector rejection. Their sum is always the original vector. @n @n
        /// If one is unsure whether @p normAxis is normalized or not, consider using @p Project instead.
        Super ProjectNorm(const Super& normAxis) const { return normAxis * Dot(normAxis); }
        /// Calculates the vector rejection with respect to the @b normalized vector @p axis, or in other words,
        /// @a only the vector components perpendicular to it are returned: <b>a⟂ = a - b̂(a·b̂)</b>
        /// @note This is the complementary to @p ProjectNorm, the vector projection. Their sum is always the original vector. @n @n
        /// If one is unsure whether @p normAxis is normalized or not, consider using @p Reject instead.
        Super RejectNorm(const Super& normAxis) const { return super() - ProjectNorm(normAxis); }
        /// Calculates the vector reflecection with respect to @p axis, or in other words,
        /// the vector component parallel to it is @a negated, while preserving all other directions:
        ///
        /// <b>proj_b̂(a) = a⟂ - a∥ = a - b̂(2a·b̂)</b>
        /// @note Equal to @code RejectNorm(normAxis) - ProjectNorm(normAxis)@endcode. @n @n
        /// If one is unsure whether @p normAxis is normalized or not, consider using @p Reflect instead.
        Super ReflectNorm(const Super& normAxis) const { return super() - normAxis * (2 * Dot(normAxis)); }

        /// Linearly interpolates in between two vectors, where @p t controls the blending:
        /// <b>lerp(a, b, t) = a + t(a-b)</b>
        /// @note @p t values outside the typical @code [0,1]@endcode are also valid.
        Super Lerp(const Super& other, T t) const { return super() + t * (other - super()); }
        /// Lerps toward a vector, aka computes the lerp and then assigns it to itself:
        /// <b>a += t(b - a)</b>
        /// @note See @p Lerp .
        Super& LerpToward(const Super& other, T t) requires Floating<T> { return super() += t * (other - super()); }
        /// Steps some distance @p d towards another vector @p other, while not overshooting if the distance is less than @p d .
        Super& StepToward(const Super& other, T d) requires Floating<T> { const auto [dir, s] = (other - super()).NormAndLen(); return AddAssign(dir * std::min(s, d)); }
        /// Steps the @a exact distance @p d towards another vector @p other, overshooting <em>even if</em> the distance is less than @p d .
        /// @note To prevent overshooting, use @p StepToward instead.
        Super& StepExact(const Super& other, T d) requires Floating<T> { return AddAssign((other - super()).Norm(d)); }
        /// Spherically interpolates between two vectors, aka interpolating along a sphere,
        /// taking the shortest past possible and moving uniformly:
        ///
        /// <b>given cos(Ω) = a·b,</b> @n
        /// <b>slerp(a, b, t) = (sin[(1-t)Ω]a + sin[tΩ]b)/sin(Ω)</b>
        ///
        /// @note Requires both vectors to be normalized.
        /// @warning Loss of precision may occur when both vectors are near opposite of each other (180°)
        Super Slerp(const Super& other, fT t) const requires Floating<T> {
            const fT theta = std::acos(Dot(other));
            const fT inv = 1 / std::sin(theta);
            return BinaryMap(other, [p = std::sin(theta * (1 - t)) * inv, q = std::sin(theta * t) * inv] (T a, T b) {
                return a * p + b * q;
            });
        }

        /// Measures the @a unsigned angle between two vectors: <b>arccos[a·b/√(‖a‖²‖b‖²)]</b>
        /// @note For 2D vectors, use @p SignedAngleBetween instead (does not require a sqrt). @n @n
        /// If both vectors are normalized beforehand, use @p AngleBetweenNorm instead.
        fT AngleBetween(const Super& other) const { return std::acos(Dot(other) / std::sqrt(LenSq() * other.LenSq())); }
        /// Measures the @a unsigned angle between two @b normalized vectors: <b>arccos(â·b̂)</b>
        /// @note For 2D vectors, use @p SignedAngleBetween instead (does not require a sqrt).
        fT AngleBetweenNorm(const Super& other) const { return std::acos(Dot(other)); }

        /// Clamps all elements in the vector to the [0, 1] range. Identical to @code Clamp(0, 1)@endcode.
        Super Clamp() const { return Clamp(0, 1); }
        /// Clamps all elements in the vector to the [@p min, @p max] range.
        Super Clamp(T min, T max) const { return Map([&] (T x) { return Math::Clamp(x, min, max); }); }
        /// Clamps the vector into the bounding box specified by @p rect .
        Super Clamp(const Rect<T, N>& rect) const;

        /// Describes and applies a linear transformation (translation + scaling) to the vector,
        /// where the transformation maps @p source to @p dest .
        Super MapCoords(const Rect<T, N>& source, const Rect<T, N>& dest) const;
        /// Describes and applies a linear transformation (translation + scaling) to the vector,
        /// where the transformation maps the unit range [0,1]ⁿ to @p dest .
        Super MapFromUnit(const Rect<T, N>& dest) const;
        /// Describes and applies a linear transformation (translation + scaling) to the vector,
        /// where the transformation maps @p source to unit range [0,1]ⁿ.
        Super UnmapToUnit(const Rect<T, N>& source) const;

        /// Clamps the length of the vector to @p d, preserving it if the length is less than @p d .
        Super ClampLen(T d) const requires Floating<T> {
            return LenSq() > d * d ? Norm(d) : super();
        }

        /// Takes the maximum component-wise on two vectors @p x and @p y .
        static Super Max(const Super& x, const Super& y) { return x.BinaryMap(y, [] (T a, T b) { return std::max(a, b); }); }
        /// Takes the minimum component-wise on two vectors @p x and @p y .
        static Super Min(const Super& x, const Super& y) { return x.BinaryMap(y, [] (T a, T b) { return std::min(a, b); }); }
        /// Takes the maximum component-wise on all the vectors within a collection @p xs .
        static Super MaxOver(const Collection<Super> auto& xs) {
            Super max = { NumInfo<T>::MIN };
            for (const auto& x : xs) max = Max(max, x);
            return max;
        }
        /// Takes the minimum component-wise on all the vectors within a collection @p xs .
        static Super MinOver(const Collection<Super> auto& xs) {
            Super min = { NumInfo<T>::MAX };
            for (const auto& x : xs) min = Min(min, x);
            return min;
        }

        /// Tests for vector equality.
        bool operator==(const IVector&) const = default;
        /// Tests whether each of the components in the vector are <em>all greater</em> than those of @p other .
        bool AllGreater(const Super& other) const {
            for (usize i = 0; i < N; ++i) if (At(i) <= other[i]) return false;
            return true;
        }
        /// Tests whether each of the components in the vector are <em>all less</em> than those of @p other .
        bool AllLess(const Super& other) const {
            for (usize i = 0; i < N; ++i) if (At(i) >= other[i]) return false;
            return true;
        }
        /// Tests whether each of the components in the vector are <em>all greater or equal</em> less than those of @p other .
        bool AllGreaterEq(const Super& other) const {
            for (usize i = 0; i < N; ++i) if (At(i) < other[i]) return false;
            return true;
        }
        /// Tests whether each of the components in the vector are <em>all less or equal</em> less than those of @p other .
        bool AllLessEq(const Super& other) const {
            for (usize i = 0; i < N; ++i) if (At(i) > other[i]) return false;
            return true;
        }
        /// Tests whether the vector lies within the bounding box @p rect .
        bool IsIn(const Rect<T, N>& rect) const;

        /// Generates a random vector through @p rand within the range given by @p range .
        /// By default, samples from the unit range [0, 1]ⁿ.
        static Super Random(RandomGenerator& rand, const Rect<T, N>& range = { 0, 1 });
        /// Generates a random vector that lies @a on the unit sphere through @p rand with a uniform distribution (∂Bⁿ),
        /// aka a vector with a length of one in a random direction.
        static Super RandomOnUnit(RandomGenerator& rand) requires Floating<T>;
        /// Generates a random vector that lies @a inside the unit sphere through @p with a uniform distribution (Bⁿ),
        /// aka a vector with a length less than one in a random direction.
        static Super RandomInUnit(RandomGenerator& rand) requires Floating<T>;

        /// Tries and parses a string of the form @code "X@Y@Z"@endcode into a vector,
        /// where @p 'X', @p 'Y', and so forth are numeric strings and @p '@' is some separator.
        /// @param s The string to parse.
        /// @param sep The separator between elements. (commonly ", ")
        /// @return Some(...) if parsing was successful, None otherwise.
        static Option<Super> Parse(Str s, Str sep) { return Parse(s, sep, [] (Str x) { return Text::Parse<T>(x); }); }
        /// Tries and parses a string of the form @code "X@Y@Z"@endcode into a vector,
        /// where @p 'X', @p 'Y', and so forth are numeric strings which will be passed to a custom parser,
        /// and @p '@' is some separator.
        /// @param s The string to parse.
        /// @param sep The separator between elements. (commonly ", ")
        /// @param parserForElms The parser for single number values which take those number strings as input. @n @n
        /// May or may not fail, indicated by None. Nulls get propagated through and cause the entire function to fail.
        /// @return Some(...) if parsing was successful, None otherwise.
        static Option<Super> Parse(Str s, Str sep, Fn<Option<T>, Str> auto&& parserForElms) {
            Super v;
            for (const auto [i, e] : s.Split(sep).Enumerate()) {
                if (i >= N) return nullptr;
                Option<T> elm = parserForElms(e);
                if (!elm) return nullptr;
                v[i] = *elm;
            }
            return v;
        }
    };

    template <class T, usize N> Vector<T, N> operator+(NoInfer<T> val, const Vector<T, N>& vec) { return vec + val; }
    template <class T, usize N> Vector<T, N> operator-(NoInfer<T> val, const Vector<T, N>& vec) { return vec.Map(val, [] (T x, T v) { return (T)(v - x); }); }
    template <class T, usize N> Vector<T, N> operator*(NoInfer<T> val, const Vector<T, N>& vec) { return vec * val; }
    template <class T, usize N> Vector<T, N> operator/(NoInfer<T> val, const Vector<T, N>& vec) { return vec.Map(val, [] (T x, T v) { return (T)(v / x); }); }

    /// <em>Not to be confused with</em> @code Vec<T>@endcode.
    /// A mathematcial vector that holds a certain number of values and represent
    /// multidimensional objects and can support various mathematical operations.
    /// @tparam T The element type.
    /// @tparam N The number of dimensions.
    template <class T, usize N> struct Vector : IVector<T, N> {
        Array<T, N> elems;
    protected:
        T* DataImpl() { return elems.Data(); }
        const T* DataImpl() const { return elems.Data(); }
        static usize CompFromNameImpl(char c) { return c == '\0' ? -1ULL : (usize)(c - 1); }
    public:
        Vector(T base = 0) { elems.Fill(base); }
        Vector(const Array<T, N>& array) : elems(array) {}

        bool operator==(const Vector&) const = default;
    };

    template <class T> struct Vector<T, 1> : IVector<T, 1> {
        friend IVector<T, 1>;
        T x;
    protected:
        T* DataImpl() { return &x; }
        const T* DataImpl() const { return &x; }
        static usize CompFromNameImpl(char c) {
            switch (c) {
                case 'x': case 's': case 'r': return 0;
                default: return -1;
            }
        }
    public:
        Vector(T x = 0) : x(x) {}

        operator const T&() const { return x; }
        operator T&() { return x; }
        bool operator==(const Vector&) const = default;
        bool operator> (T y) const { return x >  y; }
        bool operator>=(T y) const { return x >= y; }
        bool operator<=(T y) const { return x <= y; }
        bool operator< (T y) const { return x <  y; }
        T& operator++()    { return ++x; }
        T  operator++(int) { return x++; }
    };

    template <Numeric T> struct Vector<T, 2> : IVector<T, 2> {
        friend IVector<T, 2>;
        using fT = IVector<T, 2>::fT;
        T x, y;
    protected:
        T* DataImpl() { return &x; }
        const T* DataImpl() const { return &x; }
        static usize CompFromNameImpl(char c) {
            switch (c) {
                case 'x': case 's': case 'r': return 0;
                case 'y': case 't': case 'g': return 1;
                default: return -1;
            }
        }
    public:
        /// Creates the vector (s, s)
        Vector(T s = 0) : x(s), y(s) {}
        /// Creates the vector (x, y)
        Vector(T x, T y) : x(x), y(y) {}

        /// The X basis vector, x̂ = (1, 0)
        static Vector XHat() { return { 1, 0 }; }
        /// The Y basis vector, ŷ = (0, 1)
        static Vector YHat() { return { 0, 1 }; }

        /// The unit vector on the +X axis, (+1, 0)
        static Vector XPos() { return { +1, 0 }; }
        /// The unit vector on the -X axis, (-1, 0)
        static Vector XNeg() requires Signed<T> { return { -1, 0 }; }
        /// The unit vector on the +Y axis, (0, +1)
        static Vector YPos() { return { 0, +1 }; }
        /// The unit vector on the -Y axis, (0, -1)
        static Vector YNeg() requires Signed<T> { return { 0, -1 }; }

        /// The 'right' unit vector, (+1, 0)
        static Vector Right() { return XPos(); }
        /// The 'left' unit vector, (-1, 0)
        static Vector Left() requires Signed<T> { return XNeg(); }
        /// The 'up' unit vector, (0, +1)
        static Vector Up() { return YPos(); }
        /// The 'down' unit vector, (0, -1)
        static Vector Down() requires Signed<T> { return YNeg(); }

        /// Computes the 2D cross product: @n <b>a×b = a.x*b.y - a.y*b.z = ‖a‖‖b‖sinθ</b>.
        /// @note From a mathematical standpoint, this is the z-component of the cross product or the wedge product. @n @n
        /// From a geometrical standpoint, this is the signed area of the parallelogram formed by the two vectors.
        T Cross(const Vector& other) const { return x * other.y - y * other.x; }

        /// Returns a new vector with the x-component negated.
        Vector FlipX() const { return { -x, y }; }
        /// Returns a new vector with the y-component negated.
        Vector FlipY() const { return { x, -y }; }
        /// Returns the 'slope' of the vector, as if it were a line, <b>y/x</b>.
        fT Slope() const { return (fT)y / (fT)x; }
        /// Returns the 'aspect ratio' of the vector, as if it were the dimensions for a rectangle, <b>x/y</b>.
        fT AspectRatio() const { return (fT)x / (fT)y; }

        /// Returns the (polar) angle of the vector in radians, counting counter-clockwise and
        /// starting from the X unit vector (1, 0). Ignores size. @n
        /// <b>∠v = θ => ‖v‖cosθ = x & ‖v‖sinθ = y</b>. <em>(θ = atan2(y, x))</em>
        fT PolarAngle() const { return std::atan2(y, x); }
        /// Returns the @a signed angle between this vector and @p other in radians,
        /// where positive angles indicate a @a counter-clockwise rotation relative to this vector (on the @a left side);
        /// and negative angles indicate a @a clockwise rotation (on the @a right side).
        ///
        /// <b>∠(a, b) = θ => a·b = ‖a‖‖b‖cosθ & a×b = ‖a‖‖b‖sinθ</b>. <em>(θ = atan2(a×b, a·b))</em>
        /// @note Does not require both vectors to be normalized and is generally faster @a and gives more information
        /// compared to @p AngleBetween . @p SignedAngleBetween should generally @a always be prefered.
        fT SignedAngleBetween(const Vector& other) const { return std::atan2(Cross(other), this->Dot(other)); }

        /// Returns the vector in polar coordinates: (length, angle) pair,
        /// where (1, 0) subtends an angle of 0 and angles are measured in radians.
        ///
        /// <b>(r, θ) = (‖v‖, atan2(y, x))</b>
        Tuple<fT, fT> PolarCoords() const { return { this->Len(), this->PolarAngle() }; }
        /// Converts polar coordinates into a 2D vector on the Cartesian plane.
        ///
        /// <b>(x, y) = (rcosθ, rsinθ)</b>
        /// @param r The length of the vector.
        /// @param theta The angle of the vector (in radians), where (1, 0) has an angle of 0 and counts counter-clockwise.
        /// @note The angle can also be specified as a @em rotor, and should generally be prefered so if available.
        static Vector FromPolar(T r, fT theta) requires Floating<T> { return { r * std::cos(theta), r * std::sin(theta) }; }
        /// Converts polar coordinates into a 2D vector on the Cartesian plane <b>from a rotor</b>.
        ///
        /// <b>(x, y) = (rcosθ, rsinθ)</b>
        /// @param r The length of the vector.
        /// @param theta The rotor to the vector starting at (1, 0).
        static Vector FromPolar(T r, const Rotor2D& theta) requires Floating<T>;

        /// Finds the vector perpendicular to this one.
        /// @param left A boolean flag specifying whether to choose the perpendicular vector to the
        /// @a left <b>(-y, x)</b> or the @a right <b>(y, -x)</b>. Chooses the left by default.
        Vector Perpend(bool left = true) const { return { left ? -y : y, left ? x : -x }; }
        /// Finds the vector perpendicular and to the left of this one. Equal to @p Perpend(true) .
        Vector PerpendLeft() const { return { -y, x }; }
        /// Finds the vector perpendicular and to the right of this one. Equal to @p Perpend(false) .
        Vector PerpendRight() const { return { y, -x }; }

        /// Rotates the vector by some rotor @p r .
        Vector RotateBy(const Rotor2D& r) const;
        /// Transforms the vector by some transformation @p t .
        Vector TransformBy(const Transform2D& t) const;

        /// Treats both vectors as complex numbers and returns their product as a vector.
        /// @note Especially useful for geometric operations; although one may consider using @p Complex<T> in the first place.
        Vector ComplexMul(const Vector& v) const { return ComplexMul(v.x, v.y); }
        /// Treats this vector as a complex number and returns its product with @p z as a vector.
        Vector ComplexMul(const Complex<T>& z) const { return ComplexMul(z.re, z.im); }
        /// Treats this vector as a complex number and returns its product with @p u+vi as a vector.
        Vector ComplexMul(T u, T v) const { return { x * u - y * v, x * v + y * u }; }

        /// Computes the geometric product in geometric algebra for two vectors,
        /// and puts the real part in the x-component and the imaginary part in the y-component.
        /// In other words, computes the dot and cross product at once at packages them into a vector.
        ///
        /// <b>ab = a·b + a∧b</b>
        Vector GeometricMul(const Vector& v) const { return GeometricMul(v.x, v.y); }
        /// Computes the geometric product in geometric algebra for this vector and the vector (u, v),
        /// and puts the real part in the x-component and the imaginary part in the y-component.
        /// In other words, computes the dot and cross product at once at packages them into a vector.
        ///
        /// <b>(x, y)(u, v) = (xu + yv) + (xv - yu)i</b>
        Vector GeometricMul(T u, T v) const { return { x * u + y * v, x * v - y * u }; }

        /// Creates a new vector with an added z-component.
        Vec3<T> AddZ(T z) const { return { x, y, z }; }

        bool operator==(const Vector&) const = default;
};

    template <class T> struct Vector<T, 2> { T x, y; };

    template <Numeric T> struct Vector<T, 3> : IVector<T, 3> {
        friend IVector<T, 3>;
        using fT = IVector<T, 2>::fT;
        T x, y, z;
    protected:
        T* DataImpl() { return &x; }
        const T* DataImpl() const { return &x; }
        static usize CompFromNameImpl(char c) {
            switch (c) {
                case 'x': case 's': case 'r': return 0;
                case 'y': case 't': case 'g': return 1;
                case 'z': case 'u': case 'b': return 2;
                default: return -1;
            }
        }
    public:
        // Creates the vector (s, s, s)
        Vector(T s = 0) : x(s), y(s), z(s) {}
        // Creates the vector (x, y, z)
        Vector(T x, T y, T z) : x(x), y(y), z(z) {}

        /// The X basis vector, x̂ = (1, 0, 0)
        static Vector XHat() { return { 1, 0, 0 }; }
        /// The Y basis vector, ŷ = (0, 1, 0)
        static Vector YHat() { return { 0, 1, 0 }; }
        /// The Z basis vector, ẑ = (0, 0, 1)
        static Vector ZHat() { return { 0, 0, 1 }; }

        /// The unit vector on the +X axis, (+1, 0, 0)
        static Vector XPos() { return { +1, 0, 0 }; }
        /// The unit vector on the -X axis, (-1, 0, 0)
        static Vector XNeg() requires Signed<T> { return { -1, 0, 0 }; }
        /// The unit vector on the +Y axis, (0, +1, 0)
        static Vector YPos() { return { 0, +1, 0 }; }
        /// The unit vector on the -Y axis, (0, -1, 0)
        static Vector YNeg() requires Signed<T> { return { 0, -1, 0 }; }
        /// The unit vector on the +Z axis, (0, 0, +1)
        static Vector ZPos() { return { 0, 0, +1 }; }
        /// The unit vector on the -Z axis, (0, 0, -1)
        static Vector ZNeg() requires Signed<T> { return { 0, 0, -1 }; }

        /// The 'right' unit vector, (+1, 0, 0)
        static Vector Right() { return XPos(); }
        /// The 'left' unit vector, (-1, 0, 0)
        static Vector Left() requires Signed<T> { return XNeg(); }
        /// The 'up' unit vector, (0, +1, 0)
        static Vector Up() { return YPos(); }
        /// The 'down' unit vector, (0, -1, 0)
        static Vector Down() requires Signed<T> { return YNeg(); }
        /// The 'front' unit vector (+Z), (0, 0, +1)
        static Vector Front() { return ZPos(); }
        /// The 'back' unit vector (-Z), (0, 0, -1)
        static Vector Back() requires Signed<T> { return ZNeg(); }

        /// Returns a new vector with the x-component negated.
        Vector FlipX() const { return { -x, y, z }; }
        /// Returns a new vector with the y-component negated.
        Vector FlipY() const { return { x, -y, z }; }
        /// Returns a new vector with the z-component negated.
        Vector FlipZ() const { return { x, y, -z }; }

        /// Computes the cross product between two vectors, <b>a×b</b>
        Vector Cross(const Vector& other) const {
            return {
                (T)(y * other.z - z * other.y),
                (T)(z * other.x - x * other.z),
                (T)(x * other.y - y * other.x),
            };
        }

        /// Finds an @a arbitrary perpendicular vector in 3D-space from the original vector.
        /// Useful for introducing a simple helper vector.
        /// @note The returned vector is @b not guarenteed to be normalized, even if the input @b is.
        /// For normalized perpendicular vectors, see @p AnyTangent . @n @n
        /// This operation is branch-free, sqrt-free, trig-free and
        /// @b never fails/returns the zero vector (unless the vector itself is zero).
        Vector AnyPerpend() const requires Floating<T> {
            // https://math.stackexchange.com/a/4112622
            return {
                NumInfo<T>::CopySign(z, x),
                NumInfo<T>::CopySign(z, y),
                -NumInfo<T>::CopySign(std::abs(x) + std::abs(y), z),
            };
        }
        /// Finds an @a arbitrary @b normalized perpendicular vector in 3D-space from the original @b normalized vector.
        /// Useful for introducing a simple helper vector.
        /// @note The vector itself <b>must be</b> normalized.
        /// The returned vector is @b guarenteed to be normalized.
        /// For perpendicular vectors that don't need to be normalized, see @p AnyPerpend . @n @n
        /// This operation is branch-free, sqrt-free, trig-free and
        /// @b never fails/returns the zero vector.
        Vector AnyTangent() const requires Floating<T> {
            const T sign = std::copysign(1, z), a = -1 / (sign + z);
            return { x * y * a, sign + y * y * a, -y };
        }

        /// Given a @b normalized vector, generates a orthonormal basis with it being one of the basis vectors.
        /// The other @b two vectors are returned and are @b guarenteed to be @a perpendicular @b and @a normalized.
        /// @note The vector itself @b must be normalized. This function is an extension of @p AnyTangent,
        /// which gives only one perpendicular vector. @n @n
        /// This operation is branch-free, sqrt-free, trig-free and @b never fails/returns zero vectors.
        Tuple<Vector, Vector> AnyOrthoBasis() const requires Floating<T> {
            // https://graphics.pixar.com/library/OrthonormalB/paper.pdf
            const T sign = std::copysign(1, z), a = -1 / (sign + z), b = x * y * a;
            return { Vector { 1 + sign * x * x * a, sign * b, -sign * x },
                     Vector { b, sign + y * y * a, -y } };
        }

        /// Finds the @a signed angle (in radians) between this vector and @p other that lie
        /// on a common plane defined by @p normal .
        /// The direction of the angle is determined by the right hand rule using @p normal as the axis.
        /// @note Requires @p normal to be @b normalized @a and @b perpendicular to the two other vectors. @n @n
        /// One could find the normal vector by taking the cross product and normalizing it,
        /// but if that is the case the result will @a always be positive and @p AngleBetween should be prefered.
        fT SignedAngleBetween(const Vector& other, const Vector& normal) const {
            return std::atan2((fT)Cross(other).Dot(normal), (fT)this->Dot(other));
        }

        /// Returns the yaw of the vector in spherical coordinates (Y up) in radians,
        /// or in other words the angle the vector forms on the xz-plane starting at (1, 0, 0). @n
        /// <b>φ = atan2(z, x)</b>
        fT Yaw() const { return std::atan2(x, z); }
        /// Returns the pitch of the vector in spherical coordinates (Y up) in radians,
        /// or in other words the angle between the Y-axis and the vector itself. @n
        /// <b>θ = asin(y / √x²+z²)</b>
        /// @note The return value lies in the range [-π/2, π/2]
        fT Pitch() const { return std::asin(y / this->Len()); }
        /// Returns the vector in spherical coordintes in a tuple of (length, pitch, yaw) or (r, θ, φ). @n
        /// <b>(r, θ, φ) = (‖v‖, asin(y/‖v‖), atan2(z, x))</b>
        Tuple<fT, fT, fT> SphericCoords() const requires Floating<T> {
            const fT len = this->Len();
            return { len, std::asin(y / len), Yaw() };
        }
        /// Converts spherical coordinates (@p r, @p pitch = θ, @p yaw = φ) to cartesian coordintes (Y up). @n
        /// <b>(x, y, z) = (rcosθcosφ, rsinθ, rcosθsinφ)</b>
        /// @param r The length of the vector.
        /// @param pitch The pitch (angle between it and the +Y-axis) of the vector.
        /// @param yaw The yaw (angle on the xz-plane) of the vector.
        /// @note The angles can also be specified as a @em rotor, and should generally be prefered so if available.
        static Vector FromSpheric(T r, fT pitch, fT yaw) requires Floating<T> {
            const T xz = r * std::cos(pitch);
            return { xz * std::cos(yaw), r * std::sin(pitch), xz * std::sin(yaw) };
        }
        /// Converts spherical coordinates (@p r, @p pitch = θ, @p yaw = φ) to cartesian coordintes (Y up). @n
        /// <b>(x, y, z) = (rcosθcosφ, rsinθ, rcosθsinφ)</b>
        /// @param r The length of the vector.
        /// @param pitch The pitch rotor (rotor between it and the +Y-axis) of the vector.
        /// @param yaw The yaw rotor (rotor on the xz-plane) of the vector.
        static Vector FromSpheric(T r, const Rotor2D& yaw, const Rotor2D& pitch) requires Floating<T>;

        /// Interprets the 3D vector as a Homogeneous coordinate and converts it into its Euclidean form. @n
        /// <b>(x : y : z)_Hom = (x/z, y/z)_Eucl</b>
        Vec2<fT> UnHom() const { const fT invZ = (fT)1 / z; return { x * invZ, y * invZ }; }

        /// Creates a new vector with the z-component removed.
        Vec2<T> As2D() const { return { x, y }; }
        /// Creates a new vector with the w-component @p w added.
        Vec4<T> AddW(T w = 1) const { return { x, y, z, w }; }

        bool operator==(const Vector&) const = default;
    };

    template <class T> struct Vector<T, 3> { T x, y, z; };

    template <Numeric T> struct Vector<T, 4> : IVector<T, 4> {
        friend IVector<T, 4>;
        using fT = IVector<T, 2>::fT;
        T x, y, z, w;
    protected:
        T* DataImpl() { return &x; }
        const T* DataImpl() const { return &x; }
        static usize CompFromNameImpl(char c) {
            switch (c) {
                case 'x': case 's': case 'r': return 0;
                case 'y': case 't': case 'g': return 1;
                case 'z': case 'u': case 'b': return 2;
                case 'w': case 'v': case 'a': return 3;
                default: return -1;
            }
        }
    public:
        /// Creates the vector (s, s, s, w=0)
        Vector(T s = 0, T w = 0) : x(s), y(s), z(s), w(w) {}
        /// Creates the vector (x, y, z, w=1)
        Vector(T x, T y, T z, T w = 1) : x(x), y(y), z(z), w(w) {}
        /// Creates the vector (xyz, w=1)
        Vector(const Vector<T, 3>& xyz, T w = 1) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

        /// The X basis vector, x̂ = (1, 0, 0, 0)
        static Vector XHat() { return { 1, 0, 0, 0 }; }
        /// The Y basis vector, ŷ = (0, 1, 0, 0)
        static Vector YHat() { return { 0, 1, 0, 0 }; }
        /// The Z basis vector, ẑ = (0, 0, 1, 0)
        static Vector ZHat() { return { 0, 0, 1, 0 }; }
        /// The W basis vector, ŵ = (0, 0, 0, 1)
        static Vector WHat() { return { 0, 0, 0, 1 }; }

        /// The unit vector on the +X axis, (+1, 0, 0, 0)
        static Vector XPos() { return { +1, 0, 0, 0 }; }
        /// The unit vector on the -X axis, (-1, 0, 0, 0)
        static Vector XNeg() requires Signed<T> { return { -1, 0, 0, 0 }; }
        /// The unit vector on the +Y axis, (0, +1, 0, 0)
        static Vector YPos() { return { 0, +1, 0, 0 }; }
        /// The unit vector on the -Y axis, (0, -1, 0, 0)
        static Vector YNeg() requires Signed<T> { return { 0, -1, 0, 0 }; }
        /// The unit vector on the +Z axis, (0, 0, +1, 0)
        static Vector ZPos() { return { 0, 0, +1, 0 }; }
        /// The unit vector on the -Z axis, (0, 0, -1, 0)
        static Vector ZNeg() requires Signed<T> { return { 0, 0, -1, 0 }; }
        /// The unit vector on the +W axis, (0, 0, 0, +1)
        static Vector WPos() { return { 0, 0, 0, +1 }; }
        /// The unit vector on the -W axis, (0, 0, 0, -1)
        static Vector WNeg() requires Signed<T> { return { 0, 0, 0, -1 }; }

        /// The 'right' unit vector, (+1, 0, 0, 0)
        static Vector Right() { return XPos(); }
        /// The 'left' unit vector, (-1, 0, 0, 0)
        static Vector Left() requires Signed<T> { return XNeg(); }
        /// The 'up' unit vector, (0, +1, 0, 0)
        static Vector Up() { return YPos(); }
        /// The 'down' unit vector, (0, -1, 0, 0)
        static Vector Down() requires Signed<T> { return YNeg(); }
        /// The 'front' unit vector (+Z), (0, 0, +1, 0)
        static Vector Front() { return ZPos(); }
        /// The 'back' unit vector (-Z), (0, 0, -1, 0)
        static Vector Back() requires Signed<T> { return ZNeg(); }
        /// The 'in' unit vector (+W), (0, 0, 0, +1)
        static Vector In() { return WPos(); }
        /// The 'out' unit vector (-W), (0, 0, 0, -1)
        static Vector Out()  requires Signed<T> { return WNeg(); }

        /// Returns a new vector with the x-component negated.
        Vector FlipX() const { return { -x, y, z, w }; }
        /// Returns a new vector with the y-component negated.
        Vector FlipY() const { return { x, -y, z, w }; }
        /// Returns a new vector with the z-component negated.
        Vector FlipZ() const { return { x, y, -z, w }; }
        /// Returns a new vector with the w-component negated.
        Vector FlipW() const { return { x, y, z, -w }; }

        /// Interprets the 4D vector as a Homogeneous coordinate and converts it into its Euclidean form. @n
        /// <b>(x : y : z : w)_Hom = (x/w, y/w, z/w)_Eucl</b>
        Vec3<fT> UnHom() const { const fT invW = (fT)1 / w; return { x * invW, y * invW, z * invW }; }

        /// Creates a new vector with the w-component removed.
        Vec3<T> As3D() const { return { x, y, z }; }
        /// Creates a new vector with the z & w-component removed.
        Vec2<T> As2D() const { return { x, y }; }

        bool operator==(const Vector&) const = default;
    };

    template <class T> struct Vector<T, 4> { T x, y, z, w; };

    struct VectorFormatter {
        struct FormatOptions {
            Str elementFmt = "#: $", separator = ", ";
        };
        static FormatOptions ConfigureOptions(Str opt);
        template <class T>
        static usize FormatToInternal(Text::StringWriter sw, Span<const T> vector, Str comps, const FormatOptions& options) {
            OptionUsize identifiers[2] {};
            for (usize i = 0, j = 0; i < options.elementFmt.Length(); ++i) {
                const char c = options.elementFmt[i];
                if (c == '$' || c == '#') {
                    identifiers[j] = i;
                    if (++j == 2) break;
                }
            }
            Str sections[3] = {};
            if (identifiers[0]) {
                sections[0] = options.elementFmt.First(*identifiers[0]);
                if (identifiers[1]) {
                    sections[1] = options.elementFmt.Substr(*identifiers[0] + 1, *identifiers[1] - *identifiers[0] - 1);
                    sections[2] = options.elementFmt.Skip(*identifiers[1] + 1);
                } else {
                    sections[1] = options.elementFmt.Skip(*identifiers[0] + 1);
                }
            } else {
                sections[0] = options.elementFmt;
            }

            usize ln = (sections[0].Length() + sections[1].Length() + sections[2].Length() + options.separator.Length());
            ln *= vector.Length();
            ln -= options.separator.Length();
            for (usize i = 0; i < vector.Length(); i++) {
                if (i != 0)
                    sw.Write(options.separator);

                sw.Write(sections[0]);
                if (identifiers[0]) {
                    if (options.elementFmt[*identifiers[0]] == '#') {
                        ln += sw.Write(comps[i]);
                    } else {
                        ln += Text::FormatObjectTo(sw, vector[i]);
                    }
                }
                sw.Write(sections[1]);
                if (*identifiers[1]) {
                    if (options.elementFmt[*identifiers[1]] == '#') {
                        ln += sw.Write(comps[i]);
                    } else {
                        ln += Text::FormatObjectTo(sw, vector[i]);
                    }
                }
                sw.Write(sections[2]);
            }
            return ln;
        }
    };
}

namespace Quasi::Text {
    template <class T, usize N>
    struct Formatter<Math::Vector<T, N>> : Math::VectorFormatter {
        static usize FormatTo(StringWriter sw, const Math::Vector<T, N>& vec, const FormatOptions& options) {
            return VectorFormatter::FormatToInternal(sw, vec.AsSpan(), Str::Slice("xyzw", N), options);
        }
    };

    template <class T, usize N>
    struct Parser<Math::Vector<T, N>> {
        using ParseOptions = Str;
        static OptionUsize ParseUntil(Str s, Out<Math::Vector<T, N>&> out, Str) {
            Option<Math::Vector<T, N>> v = Math::Vector<T, N>::Parse(s, ", ");
            if (!v) return nullptr;
            out = *v;
            return s.Length();
        }
    };
}