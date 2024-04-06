#pragma once
#include <string>
#include <map>
#include <vector>
#include <core.h>
#include <ranges>
#include <stdu/macros.h>

#include "GLObject.h"
#include "Matrix.h"
#include "Color.h"
#include "Rect.h"

namespace Graphics {
    enum class ShaderType {
        VERTEX = 0x8B31,
        FRAGMENT = 0x8B30,
        GEOMETRY = 0x8DD9,
        // all below requires OpenGL 4.3 above
        COMPUTE = 0x91B9,
        TESS_CONTROL = 0x8E88,
        TESS_EVALUATION = 0x8E87,
    };

#define NAME_INT I
#define NAME_UINT UI
#define NAME_FLOAT F
#define ONE_1 1
#define ONE_2 0
#define ONE_3 0
#define ONE_4 0
#define MAKE_UNIFORM_BITMASK(T, R, M, N) (T << 7) | (R << 6) | (M << 3) | N
#define DEFINE_UNIF(T, M, N, R) \
    STDU_IF_ELSE(STDU_CAT(ONE_, M), (STDU_CAT(STDU_CAT(UNIF_##N, STDU_CAT(NAME_, T)), STDU_IF(R, _ARR))), (UNIF_MAT##M##x##N)) = \
    STDU_CAT(T, _FLAG) | ROWS_##M | COLS_##N STDU_IF(R, | ARRAY_FLAG)
#define DEFINE_WHOLE_UNIF(T) \
    DEFINE_UNIF(T, 1, 1, 0), DEFINE_UNIF(T, 1, 2, 0), DEFINE_UNIF(T, 1, 3, 0), DEFINE_UNIF(T, 1, 4, 0), \
    DEFINE_UNIF(T, 1, 1, 1), DEFINE_UNIF(T, 1, 2, 1), DEFINE_UNIF(T, 1, 3, 1), DEFINE_UNIF(T, 1, 4, 1)
    enum class ShaderUniformType : int {
        // layout of this type: TT_R_MMM_NNN
        // T: type of data, R: bool value, true if is array
        // MxN: matrix part of MatrixMxN, otherwise N is vector dimension
        TYPE_MASK   = MAKE_UNIFORM_BITMASK(0b11, 0, 0, 0),
        INT_FLAG    = MAKE_UNIFORM_BITMASK(2, 0, 0, 0),
        UINT_FLAG   = MAKE_UNIFORM_BITMASK(3, 0, 0, 0),
        FLOAT_FLAG  = MAKE_UNIFORM_BITMASK(1, 0, 0, 0),

        ARRAY_FLAG  = MAKE_UNIFORM_BITMASK(0, 0b1, 0, 0),

        ROWS_MASK   = MAKE_UNIFORM_BITMASK(0, 0, 0b111, 0),
        COLS_MASK   = MAKE_UNIFORM_BITMASK(0, 0, 0, 0b111),

        VECTOR_FLAG = MAKE_UNIFORM_BITMASK(0, 0, 0b001, 0),
        ROWS_1      = MAKE_UNIFORM_BITMASK(0, 0, 1, 0),
        ROWS_2      = MAKE_UNIFORM_BITMASK(0, 0, 2, 0),
        ROWS_3      = MAKE_UNIFORM_BITMASK(0, 0, 3, 0),
        ROWS_4      = MAKE_UNIFORM_BITMASK(0, 0, 4, 0),

        SINGLE_FLAG = MAKE_UNIFORM_BITMASK(0, 0, 0, 0b001),
        COLS_1      = MAKE_UNIFORM_BITMASK(0, 0, 0, 1),
        COLS_2      = MAKE_UNIFORM_BITMASK(0, 0, 0, 2),
        COLS_3      = MAKE_UNIFORM_BITMASK(0, 0, 0, 3),
        COLS_4      = MAKE_UNIFORM_BITMASK(0, 0, 0, 4),

        DEFINE_WHOLE_UNIF(INT),
        DEFINE_WHOLE_UNIF(UINT),
        DEFINE_WHOLE_UNIF(FLOAT),

        DEFINE_UNIF(FLOAT, 2, 2, 1), DEFINE_UNIF(FLOAT, 2, 3, 1), DEFINE_UNIF(FLOAT, 2, 4, 1),
        DEFINE_UNIF(FLOAT, 3, 2, 1), DEFINE_UNIF(FLOAT, 3, 3, 1), DEFINE_UNIF(FLOAT, 3, 4, 1),
        DEFINE_UNIF(FLOAT, 4, 2, 1), DEFINE_UNIF(FLOAT, 4, 3, 1), DEFINE_UNIF(FLOAT, 4, 4, 1),
    };
#undef NAME_INT
#undef NAME_UINT
#undef NAME_FLOAT
#undef ONE_1
#undef ONE_2
#undef ONE_3
#undef ONE_4
#undef MAKE_UNIFORM_BITMASK
#undef DEFINE_UNIF
#undef DEFINE_WHOLE_UNIF
    STDU_IMPL_ENUM_OPERATORS(ShaderUniformType);

    namespace _impl_shader_arg {
        template <class T> struct use_const_ref { using type = T; };

        template <ShaderUniformType U>
        auto impl() {
            constexpr auto type = U & ShaderUniformType::TYPE_MASK;
            constexpr bool array = (bool)(U & ShaderUniformType::ARRAY_FLAG);
            constexpr int rows = (int)((U & ShaderUniformType::ROWS_MASK) / ShaderUniformType::VECTOR_FLAG);
            constexpr int cols = (int)((U & ShaderUniformType::COLS_MASK) / ShaderUniformType::SINGLE_FLAG);

            using T = std::conditional_t<type == ShaderUniformType::INT_FLAG,   int,
                      std::conditional_t<type == ShaderUniformType::UINT_FLAG,  uint,
                      std::conditional_t<type == ShaderUniformType::FLOAT_FLAG, float, void>>>;

            if constexpr (rows > 1) {
                return std::span<const Maths::matrix<rows, cols>> {};
            } else if constexpr(cols > 1) {
                if constexpr (array) return std::span<const Maths::vecn<cols, T>> {};
                else return use_const_ref<Maths::vecn<cols, T>> {};
            } else {
                if constexpr (array) return std::span<const T> {};
                else return T {};
            }
        }

        template <class T> struct arg { using type = T; };
        template <class T> struct arg<use_const_ref<T>> { using type = const T&; };
    }

    template <ShaderUniformType U>
    using ShaderUniformArgOf = typename _impl_shader_arg::arg<decltype(_impl_shader_arg::impl<U>())>::type;

    template <class T>
    constexpr ShaderUniformType ConvertUniformType =
        std::is_same_v<T, int>   ? ShaderUniformType::INT_FLAG   :
        std::is_same_v<T, uint>  ? ShaderUniformType::UINT_FLAG  :
        std::is_same_v<T, float> ? ShaderUniformType::FLOAT_FLAG :
        (ShaderUniformType)0;

    struct ShaderProgramSource {
        std::string fullSource;
        usize sepPoints[2]; // vertex | fragment | geometery, all seperated with index.

    private:
        [[nodiscard]] int to_id(ShaderType type) const { switch (type) {
            case ShaderType::VERTEX: return 0;
            case ShaderType::FRAGMENT: return 1;
            case ShaderType::GEOMETRY: return 2;
            default: return 3;
        } }
    public:
        [[nodiscard]] usize GetSepPoint(int idx) const {
            return idx < 0 ? 0 : idx >= 2 ? fullSource.size() : sepPoints[idx];
        }

        [[nodiscard]] std::string_view GetShader(ShaderType type) const {
            const usize beg = GetSepPoint(to_id(type) - 1);
            return std::string_view { fullSource }.substr(beg, GetSepPoint(to_id(type)) - beg);
        }
    };

    struct ShaderHandler : GLObjectHandler<ShaderHandler> {
        [[nodiscard]] OPENGL_API glID Create() const;
        OPENGL_API void Destroy(glID id) const;
        OPENGL_API void Bind(glID id) const;
        OPENGL_API void Unbind() const;
    };

    struct ShaderArgs;
    struct ShaderValueVariant;
    struct ShaderParameter;

    class Shader : public GLObject<ShaderHandler> {
        std::map<std::string, int, std::less<>> uniformCache;

    public:
        Shader() = default;
        OPENGL_API explicit Shader(std::string_view program);
        OPENGL_API explicit Shader(std::string_view vert, std::string_view frag, std::string_view geom = {});

        using stringr = const std::string&; // the 'r' stands for reference

        static STDU_ENUM_TOSTR(ShaderType, ShaderTypeName,
            (VERTEX,          "Vertex")
            (FRAGMENT,        "Fragment")
            (GEOMETRY,        "Geometry")
            (COMPUTE,         "Compute")
            (TESS_CONTROL,    "Tesselation Control")
            (TESS_EVALUATION, "Tesselation Eval"),
            "Undefined")

        template <ShaderUniformType U>
        void SetUniformAtLoc(int uniformLoc, ShaderUniformArgOf<U> val) = delete;
        template <ShaderUniformType U>
        void SetUniformOf(stringr name, ShaderUniformArgOf<U> val) { SetUniformAtLoc<U>(GetUniformLoc(name), val); }

#pragma region Uniforms
#define UNIF_INSTANTIATE \
        DEFINE_UNIF_FN(Int,      1I)      DEFINE_UNIF_FN(Ivec2,    2I)      DEFINE_UNIF_FN(Ivec3,    3I)      DEFINE_UNIF_FN(Ivec4, 4I) \
        DEFINE_UNIF_FN(Uint,     1UI)     DEFINE_UNIF_FN(Uvec2,    2UI)     DEFINE_UNIF_FN(Uvec3,    3UI)     DEFINE_UNIF_FN(Uvec4, 4UI) \
        DEFINE_UNIF_FN(Float,    1F)      DEFINE_UNIF_FN(Fvec2,    2F)      DEFINE_UNIF_FN(Fvec3,    3F)      DEFINE_UNIF_FN(Fvec4, 4F) \
        DEFINE_UNIF_FN(IntArr,   1I_ARR)  DEFINE_UNIF_FN(Ivec2Arr, 2I_ARR)  DEFINE_UNIF_FN(Ivec3Arr, 3I_ARR)  DEFINE_UNIF_FN(Ivec4Arr, 4I_ARR) \
        DEFINE_UNIF_FN(UintArr,  1UI_ARR) DEFINE_UNIF_FN(Uvec2Arr, 2UI_ARR) DEFINE_UNIF_FN(Uvec3Arr, 3UI_ARR) DEFINE_UNIF_FN(Uvec4Arr, 4UI_ARR) \
        DEFINE_UNIF_FN(FloatArr, 1F_ARR)  DEFINE_UNIF_FN(Fvec2Arr, 2F_ARR)  DEFINE_UNIF_FN(Fvec3Arr, 3F_ARR)  DEFINE_UNIF_FN(Fvec4Arr, 4F_ARR) \
        \
        DEFINE_UNIF_FN(Mat2x2, MAT2x2) DEFINE_UNIF_FN(Mat2x3, MAT2x3) DEFINE_UNIF_FN(Mat2x4, MAT2x4) \
        DEFINE_UNIF_FN(Mat3x2, MAT3x2) DEFINE_UNIF_FN(Mat3x3, MAT3x3) DEFINE_UNIF_FN(Mat3x4, MAT3x4) \
        DEFINE_UNIF_FN(Mat4x2, MAT4x2) DEFINE_UNIF_FN(Mat4x3, MAT4x3) DEFINE_UNIF_FN(Mat4x4, MAT4x4) \

#define DEFINE_UNIF_FN(N, IN) \
    void SetUniform##N(stringr name, ShaderUniformArgOf<ShaderUniformType::UNIF_##IN> val) { \
        SetUniformOf<ShaderUniformType::UNIF_##IN>(name, val); \
    }

        UNIF_INSTANTIATE

        OPENGL_API void SetUniformDyn(const ShaderParameter& arg);
        OPENGL_API void SetUniformArgs(const ShaderArgs& args);

        OPENGL_API void SetUniformTex(stringr name, const class Texture& texture);

        template <class N>
        void SetUniform(stringr name, N num) {
            using enum ShaderUniformType;
            SetUniformOf<ConvertUniformType<N> | VECTOR_FLAG | SINGLE_FLAG>(name, num);
        }

        template <uint N, class T>
        void SetUniform(stringr name, const Maths::vecn<N, T>& vec) {
            using enum ShaderUniformType;
            SetUniformOf<ConvertUniformType<T> | VECTOR_FLAG | (COLS_1 * N)>(name, vec);
        }

        template <class T>
        void SetUniform(stringr name, std::span<const T> val) {
            using enum ShaderUniformType;
            SetUniformOf<ConvertUniformType<T> | VECTOR_FLAG | SINGLE_FLAG | ARRAY_FLAG>(name, val);
        }

        template <uint N, class T>
        void SetUniform(stringr name, std::span<const Maths::vecn<N, T>> val) {
            using enum ShaderUniformType;
            SetUniformOf<ConvertUniformType<T> | VECTOR_FLAG | (COLS_1 * N) | ARRAY_FLAG>(name, val);
        }

        template <uint N, uint M>
        void SetUniform(stringr name, const Maths::matrix<N, M>& val) {
            using enum ShaderUniformType;
            SetUniformOf<FLOAT_FLAG | (ROWS_1 * N) | (COLS_1 * M) | ARRAY_FLAG>(name, val.data());
        }

        void SetUniformMat2x2(stringr name, const Maths::mat2x2& mat) { SetUniformMat2x2(name, std::ranges::single_view { mat }); }
        void SetUniformMat2x3(stringr name, const Maths::mat2x3& mat) { SetUniformMat2x3(name, std::ranges::single_view { mat }); }
        void SetUniformMat2x4(stringr name, const Maths::mat2x4& mat) { SetUniformMat2x4(name, std::ranges::single_view { mat }); }
        void SetUniformMat3x2(stringr name, const Maths::mat3x2& mat) { SetUniformMat3x2(name, std::ranges::single_view { mat }); }
        void SetUniformMat3x3(stringr name, const Maths::mat3x3& mat) { SetUniformMat3x3(name, std::ranges::single_view { mat }); }
        void SetUniformMat3x4(stringr name, const Maths::mat3x4& mat) { SetUniformMat3x4(name, std::ranges::single_view { mat }); }
        void SetUniformMat4x2(stringr name, const Maths::mat4x2& mat) { SetUniformMat4x2(name, std::ranges::single_view { mat }); }
        void SetUniformMat4x3(stringr name, const Maths::mat4x3& mat) { SetUniformMat4x3(name, std::ranges::single_view { mat }); }
        void SetUniformMat4x4(stringr name, const Maths::mat4x4& mat) { SetUniformMat4x4(name, std::ranges::single_view { mat }); }
#pragma endregion

#pragma region Shader Sources
#define GLSL_SHADER(VERSION, V, F) "#shader vertex\n" "#version " #VERSION " core\n" STDU_TOSTR(STDU_REMOVE_SCOPE(V)) "\n#shader fragment\n" "#version " #VERSION " core\n" STDU_TOSTR(STDU_REMOVE_SCOPE(F))

        inline const static std::string StdColored =
            GLSL_SHADER(330,
                (
                    layout(location = 0) in vec4 position;
                    layout(location = 1) in vec4 color;
                    out vec4 v_color;
                    uniform mat4 u_projection;
                    uniform mat4 u_view;
                    void main() {
                        gl_Position = u_projection * u_view * position;
                        v_color = color;
                    }
                ),
                (
                    layout(location = 0) out vec4 color;
                    in vec4 v_color;
                    void main() {
                        color = v_color;
                    }
                )
            );

        inline const static std::string StdTextured =
            GLSL_SHADER(330,
                (
                    layout(location = 0) in vec4 position;
                    layout(location = 1) in vec4 color;
                    layout(location = 2) in vec2 texCoord;
                    out vec2 v_TexCoord;
                    out vec4 v_color;
                    uniform mat4 u_projection;
                    uniform mat4 u_view;
                    void main() {
                        gl_Position = u_projection * u_view * position;
                        v_color = color;
                        v_TexCoord = texCoord;
                    }
                ),
                (
                    layout(location = 0) out vec4 color;
                    in vec2 v_TexCoord;
                    in vec4 v_color;
                    uniform sampler2D u_Texture;
                    void main() {
                        vec4 texColor = texture(u_Texture, v_TexCoord);
                        color = v_color * texColor;
                    }
                )
            );
#pragma endregion // Shader Sources

        OPENGL_API static Shader FromFile(stringr filepath);
        OPENGL_API static Shader FromFile(stringr vert, stringr frag, stringr geom = {});

        OPENGL_API int GetUniformLoc(stringr name) { return GetUniformLocation(name); }
    private:
        OPENGL_API int GetUniformLocation(std::string_view name);
        OPENGL_API static ShaderProgramSource ParseShader(std::string_view program);
        OPENGL_API static ShaderProgramSource ParseFromFile(const std::string& filepath);
        OPENGL_API static uint CompileShader(std::string_view source, ShaderType type);
        static uint CompileShaderVert(std::string_view source) { return CompileShader(source, ShaderType::VERTEX); }
        static uint CompileShaderFrag(std::string_view source) { return CompileShader(source, ShaderType::FRAGMENT); }
        static uint CompileShaderGeom(std::string_view source) { return CompileShader(source, ShaderType::GEOMETRY); }
        OPENGL_API static uint CreateShader(std::string_view vtx, std::string_view frg, std::string_view geo = {});
    };

#undef DEFINE_UNIF_FN
#define DEFINE_UNIF_FN(N, IN) template <> OPENGL_API void Shader::SetUniformAtLoc<ShaderUniformType::UNIF_##IN>(int uniformLoc, ShaderUniformArgOf<ShaderUniformType::UNIF_##IN> val);
    UNIF_INSTANTIATE
#undef DEFINE_UNIF_FN
#undef UNIF_INSTANTIATE

    struct ShaderParameter;
    struct ShaderArgsIter;

    struct ShaderValueVariant {
        union {
            int   datInt;
            uint  datUint;
            float datFloat;
            const int*   datIntPtr;
            const uint*  datUintPtr;
            const float* datFloatPtr;
        };
        ShaderUniformType type;
        uint size;

#pragma region Init
        ShaderValueVariant(bool val) : datInt(val), type(ShaderUniformType::UNIF_1I), size(1) {}

        template <class N> requires std::is_arithmetic_v<N>
        ShaderValueVariant(N num) {
            using enum ShaderUniformType;
            type = ConvertUniformType<N> | VECTOR_FLAG | SINGLE_FLAG;
            if      constexpr (std::is_same_v<N, float>) datFloat = num;
            else if constexpr (std::is_same_v<N, int>)   datInt   = num;
            else if constexpr (std::is_same_v<N, uint>)  datUint  = num;
            size = 1;
        }

        template <Maths::vec_t V>
        ShaderValueVariant(const V& vec) {
            using enum ShaderUniformType;
            using T = typename V::scalar;
            constexpr int N = V::dimension;
            type = ConvertUniformType<T> | VECTOR_FLAG | (COLS_1 * N);
            if      constexpr (std::is_same_v<T, float>) datFloatPtr = vec.begin();
            else if constexpr (std::is_same_v<T, int>)   datIntPtr   = vec.begin();
            else if constexpr (std::is_same_v<T, uint>)  datUintPtr  = vec.begin();
            size = N;
        }

        template <class T>
        ShaderValueVariant(std::span<const T> val) {
            using enum ShaderUniformType;
            type = ConvertUniformType<T> | VECTOR_FLAG | SINGLE_FLAG | ARRAY_FLAG;
            if      constexpr (std::is_same_v<T, float>) datFloatPtr = val.begin();
            else if constexpr (std::is_same_v<T, int>)   datIntPtr   = val.begin();
            else if constexpr (std::is_same_v<T, uint>)  datUintPtr  = val.begin();
            size = (uint)val.size();
        }

        template <Maths::vec_t V>
        ShaderValueVariant(std::span<const V> val) {
            using enum ShaderUniformType;
            using T = typename V::scalar;
            constexpr int N = V::dimension;
            type = ConvertUniformType<T> | VECTOR_FLAG | (COLS_1 * N) | ARRAY_FLAG;
            if      constexpr (std::is_same_v<T, float>) datFloatPtr = (const float*)val.begin();
            else if constexpr (std::is_same_v<T, int>)   datIntPtr   = (const int*)  val.begin();
            else if constexpr (std::is_same_v<T, uint>)  datUintPtr  = (const uint*) val.begin();
            size = val.size() * N;
        }

        ShaderValueVariant(const Maths::colorf&  val) : datFloatPtr(val.begin()), type(ShaderUniformType::UNIF_4F), size(4) {}
        ShaderValueVariant(const Maths::color3f& val) : datFloatPtr(val.begin()), type(ShaderUniformType::UNIF_3F), size(3) {}
        ShaderValueVariant(std::span<const Maths::colorf>  dat) : datFloatPtr((const float*)dat.data()), type(ShaderUniformType::UNIF_4F_ARR), size(4 * (uint)dat.size()) {}
        ShaderValueVariant(std::span<const Maths::color3f> dat) : datFloatPtr((const float*)dat.data()), type(ShaderUniformType::UNIF_3F_ARR), size(3 * (uint)dat.size()) {}

        template <uint N, uint M>
        ShaderValueVariant(const Maths::matrix<N, M>& val) {
            using enum ShaderUniformType;
            type = FLOAT_FLAG | (ROWS_1 * N) | (COLS_1 * M) | ARRAY_FLAG;
            datFloatPtr = val.data().data();
            size = N * M;
        }

        template <uint N, uint M>
        ShaderValueVariant(std::span<const Maths::matrix<N, M>> data) {
            using enum ShaderUniformType;
            type = FLOAT_FLAG | (ROWS_1 * N) | (COLS_1 * M) | ARRAY_FLAG;
            datFloatPtr = (const float*)data.data();
            size = N * M * (uint)data.size();
        }

        OPENGL_API ShaderValueVariant(const Texture& tex);
#pragma endregion

    private:
        template <class> struct as_impl {};

#define DEFINE_AS_IMPL(T, ...) struct as_impl<STDU_REMOVE_SCOPE(T)> { [[nodiscard]] STDU_REMOVE_SCOPE(T) as(const ShaderValueVariant& s) const { return __VA_ARGS__; } };
        template <> DEFINE_AS_IMPL((int),          s.datInt)
        template <> DEFINE_AS_IMPL((uint),         s.datUint)
        template <> DEFINE_AS_IMPL((float),        s.datFloat)
        template <> DEFINE_AS_IMPL((const int*),   s.datIntPtr)
        template <> DEFINE_AS_IMPL((const uint*),  s.datUintPtr)
        template <> DEFINE_AS_IMPL((const float*), s.datFloatPtr)
        template <class T> DEFINE_AS_IMPL((std::span<T>), { as_impl<T*> {}.as(s), s.size })
        template <uint N, class T> DEFINE_AS_IMPL((Maths::vecn<N, T>), [&]{ auto t = as_impl<std::span<const T>> {}.as(s); return Maths::vecn<N, T>::from_span(t); }())
        template <uint N, class T> DEFINE_AS_IMPL((std::span<const Maths::vecn<N, T>>), stdu::span_cast<const Maths::vecn<N, T>>(as_impl<std::span<const T>> {}.as(s)))
        template <uint N, uint M>  DEFINE_AS_IMPL((std::span<const Maths::matrix<N, M>>), stdu::span_cast<const Maths::matrix<N, M>>(as_impl<std::span<const float>> {}.as(s)))
#undef DEFINE_AS_IMPL
    public:

        template <class T> std::decay_t<T> as() const { return as_impl<std::decay_t<T>> {}.as(*this); }

        template <class T>
        [[nodiscard]] bool is() const {
            using enum ShaderUniformType;
            const int typeId = (int)((type / (1 << 7)) * 2 + ((type & ARRAY_FLAG) / ARRAY_FLAG));
            constexpr int INT = 2, UINT = 4, FLOAT = 6, INTPTR = 3, UINTPTR = 5, FLOATPTR = 7;
            switch (typeId) {
                case INT:      return std::is_same_v<T, int>;
                case UINT:     return std::is_same_v<T, uint>;
                case FLOAT:    return std::is_same_v<T, float>;
                case INTPTR:   return std::is_same_v<T, const int*>;
                case UINTPTR:  return std::is_same_v<T, const uint*>;
                case FLOATPTR: return std::is_same_v<T, const float*>;
                default: return false;
            }
        }
    };

    struct ShaderArgs {
        struct ShaderSubParam {
            Maths::rangez subname;
            ShaderValueVariant value;
        };

        std::string nullSepName;
        std::vector<ShaderSubParam> params;

        ShaderArgs() {}
        OPENGL_API ShaderArgs(std::initializer_list<ShaderParameter> p);

        [[nodiscard]] usize size() const { return params.size(); }

        ShaderArgs& then(std::string_view name, ShaderValueVariant val) {
            const usize beg = size();
            nullSepName.append(name);
            const usize end = size();
            nullSepName.append(nullptr);

            params.emplace_back(Maths::rangez { beg, end }, val);

            return *this;
        }

        using Iter = ShaderArgsIter;

        [[nodiscard]] Iter begin() const;
        [[nodiscard]] Iter end() const;
    };

    struct ShaderParameter {
        std::string_view name;
        ShaderValueVariant value;
    };

    struct ShaderArgsIter {
        stdu::ref<const ShaderArgs> parent;
        const ShaderArgs::ShaderSubParam* ptr;

        ShaderParameter operator*() const {
            return {
                std::string_view { parent->nullSepName }.substr(ptr->subname.min, ptr->subname.width()),
                ptr->value
            };
        }

        ShaderArgsIter& operator++() { ++ptr; return *this; }
        bool operator==(ShaderArgsIter other) const { return ptr == other.ptr; }
    };

    inline ShaderArgs::Iter ShaderArgs::begin() const { return { .parent = *this, .ptr = params.data() }; }
    inline ShaderArgs::Iter ShaderArgs::end()   const { return { .parent = *this, .ptr = params.data() + params.size() }; }
}

template <>
struct std::formatter<Graphics::ShaderType> : std::formatter<std::string> {
    auto format(Graphics::ShaderType s, std::format_context& ctx) const {
        return std::formatter<std::string>::format(Graphics::Shader::ShaderTypeName(s), ctx);
    }
};