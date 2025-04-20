#pragma once

#include <Utils/Macros.h>

#include "Utils/Enum.h"
#include "Utils/Array.h"
#include "Utils/String.h"
#include "Utils/CStr.h"
#include "GLObject.h"
#include "Math/Matrix.h"
#include "Math/Color.h"
#include "Utils/HashMap.h"

namespace Quasi::Graphics {
    struct ShaderTypeData {
        u32 glID;
        Str shaderName;

        QDefineEnum$(ShaderType,
            (VERTEX,   (0x8B31, "Vertex"))
            (FRAGMENT, (0x8B30, "Fragment"))
            (GEOMETRY, (0x8DD9, "Geometry"))
            // all below requires OpenGL 4.3 above
            (COMPUTE,         (0x91B9, "Compute"))
            (TESS_CONTROL,    (0x8E88, "Tesselation Control"))
            (TESS_EVALUATION, (0x8E87, "Tesselation Eval")),
        NONE)
    };

    enum class ShaderUniformType {
        // layout of this type: R_TT_MM_NN
        // T: type of data, R: bool value, true if is array
        // MxN: matrix part of Matrix(M+1)x(N+1), otherwise N+1 is vector dimension
        F_UNIT    = 0x00, FV2       = 0x01, FV3       = 0x02, FV4       = 0x03,
        FMAT_2X2  = 0x05, FMAT_2X3  = 0x06, FMAT_2X4  = 0x07,
        FMAT_3X2  = 0x09, FMAT_3X3  = 0x0A, FMAT_3X4  = 0x0B,
        FMAT_4X2  = 0x0D, FMAT_4X3  = 0x0E, FMAT_4X4  = 0x0F,
        F_ARRAY   = 0x40, FV2_ARRAY = 0x41, FV3_ARRAY = 0x42, FV4_ARRAY = 0x43,
        I_UNIT    = 0x10, IV2       = 0x11, IV3       = 0x12, IV4       = 0x13,
        I_ARRAY   = 0x50, IV2_ARRAY = 0x51, IV3_ARRAY = 0x52, IV4_ARRAY = 0x53,
        U_UNIT    = 0x20, UV2       = 0x21, UV3       = 0x22, UV4       = 0x23,
        U_ARRAY   = 0x60, UV2_ARRAY = 0x61, UV3_ARRAY = 0x62, UV4_ARRAY = 0x63,
    };

    struct ShaderArgs;
    struct ShaderParameter;

    class Shader : public GLObject<Shader> {
        HashMap<String, int> uniformCache;

        explicit Shader(GraphicsID id);
    public:
        Shader() = default;
        static Shader New(Str program);
        static Shader New(Str vert, Str frag, Str geom = {});
        static void DestroyObject(GraphicsID id);
        static void BindObject(GraphicsID id);
        static void UnbindObject();

        void SetUniformDyn(CStr name, ShaderUniformType type, Bytes data);
        void SetUniformArgs(const ShaderArgs& args);

#pragma region Shader Uniform Types
        void SetUniformFloat(CStr name, float x);
        void SetUniformFv2(CStr name, const Math::fVector2& v2s);
        void SetUniformFv3(CStr name, const Math::fVector3& v3s);
        void SetUniformFv4(CStr name, const Math::fVector4& v4s);
        void SetUniformFloatArr(CStr name, Span<const float> xs);
        void SetUniformFv2Arr(CStr name, Span<const Math::fVector2> v2s);
        void SetUniformFv3Arr(CStr name, Span<const Math::fVector3> v3s);
        void SetUniformFv4Arr(CStr name, Span<const Math::fVector4> v4s);
        void SetUniformInt(CStr name, int x);
        void SetUniformIv2(CStr name, const Math::iVector2& v2s);
        void SetUniformIv3(CStr name, const Math::iVector3& v3s);
        void SetUniformIv4(CStr name, const Math::iVector4& v4s);
        void SetUniformIntArr(CStr name, Span<const int> xs);
        void SetUniformIv2Arr(CStr name, Span<const Math::iVector2> v2s);
        void SetUniformIv3Arr(CStr name, Span<const Math::iVector3> v3s);
        void SetUniformIv4Arr(CStr name, Span<const Math::iVector4> v4s);
        void SetUniformUint(CStr name, uint x);
        void SetUniformUv2(CStr name, const Math::uVector2& v2s);
        void SetUniformUv3(CStr name, const Math::uVector3& v3s);
        void SetUniformUv4(CStr name, const Math::uVector4& v4s);
        void SetUniformUintArr(CStr name, Span<const uint> xs);
        void SetUniformUv2Arr(CStr name, Span<const Math::uVector2> v2s);
        void SetUniformUv3Arr(CStr name, Span<const Math::uVector3> v3s);
        void SetUniformUv4Arr(CStr name, Span<const Math::uVector4> v4s);

        void SetUniformColor(CStr name, const Math::fColor3& color3);
        void SetUniformColor(CStr name, const Math::fColor&  color);
        void SetUniformTex(CStr name, const class Texture& texture);

        void SetUniformMat2x2(CStr name, const Math::Matrix2x2& mat) { SetUniformMat2x2Arr(name, Spans::Only(mat)); }
        void SetUniformMat2x3(CStr name, const Math::Matrix2x3& mat) { SetUniformMat2x3Arr(name, Spans::Only(mat)); }
        void SetUniformMat2x4(CStr name, const Math::Matrix2x4& mat) { SetUniformMat2x4Arr(name, Spans::Only(mat)); }
        void SetUniformMat3x2(CStr name, const Math::Matrix3x2& mat) { SetUniformMat3x2Arr(name, Spans::Only(mat)); }
        void SetUniformMat3x3(CStr name, const Math::Matrix3x3& mat) { SetUniformMat3x3Arr(name, Spans::Only(mat)); }
        void SetUniformMat3x4(CStr name, const Math::Matrix3x4& mat) { SetUniformMat3x4Arr(name, Spans::Only(mat)); }
        void SetUniformMat4x2(CStr name, const Math::Matrix4x2& mat) { SetUniformMat4x2Arr(name, Spans::Only(mat)); }
        void SetUniformMat4x3(CStr name, const Math::Matrix4x3& mat) { SetUniformMat4x3Arr(name, Spans::Only(mat)); }
        void SetUniformMat4x4(CStr name, const Math::Matrix4x4& mat) { SetUniformMat4x4Arr(name, Spans::Only(mat)); }
        void SetUniformMat2x2Arr(CStr name, Span<const Math::Matrix2x2> mats);
        void SetUniformMat2x3Arr(CStr name, Span<const Math::Matrix2x3> mats);
        void SetUniformMat2x4Arr(CStr name, Span<const Math::Matrix2x4> mats);
        void SetUniformMat3x2Arr(CStr name, Span<const Math::Matrix3x2> mats);
        void SetUniformMat3x3Arr(CStr name, Span<const Math::Matrix3x3> mats);
        void SetUniformMat3x4Arr(CStr name, Span<const Math::Matrix3x4> mats);
        void SetUniformMat4x2Arr(CStr name, Span<const Math::Matrix4x2> mats);
        void SetUniformMat4x3Arr(CStr name, Span<const Math::Matrix4x3> mats);
        void SetUniformMat4x4Arr(CStr name, Span<const Math::Matrix4x4> mats);
#pragma endregion

#pragma region Shader Sources
#define Q_GLSL_SHADER(VERSION, V, F) "#shader vertex\n" "#version " #VERSION " core\n" Q_TOSTR(Q_REMOVE_SCOPE(V)) "\n#shader fragment\n" "#version " #VERSION " core\n" Q_TOSTR(Q_REMOVE_SCOPE(F))

        static constexpr Str StdColored =
            Q_GLSL_SHADER(330,
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

        static constexpr Str StdTextured =
            Q_GLSL_SHADER(330,
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

        static Shader FromFile(CStr filepath);
        static Shader FromFile(CStr vert, CStr frag, CStr geom = {});

        static bool IsArrayUnif(ShaderUniformType type);
        static bool IsMatrixUnif(ShaderUniformType type);
    private:
        int GetUniformLocation(CStr name);
        static Tuple<Str, Str, Str> ParseShader  (Str program);
        static GraphicsID CompileShader    (Str source, ShaderType type);
        static GraphicsID CompileShaderVert(Str source) { return CompileShader(source, ShaderType::VERTEX); }
        static GraphicsID CompileShaderFrag(Str source) { return CompileShader(source, ShaderType::FRAGMENT); }
        static GraphicsID CompileShaderGeom(Str source) { return CompileShader(source, ShaderType::GEOMETRY); }
        static GraphicsID CreateShader(Str vtx, Str frg, Str geo = {});

        friend class GraphicsDevice;
    };

    struct ShaderParameter;

    struct ShaderParameter {
        using enum ShaderUniformType;
        Str name;
        ShaderUniformType utype;
        RawUnion<Array<f32, 4>, Array<u32, 4>, Bytes> payload;

        ShaderParameter(Str name, ShaderUniformType utype, const u32 (&p) [4]) : name(name), utype(utype) { payload.InitWith<Array<u32, 4>>(p); }
        ShaderParameter(Str name, ShaderUniformType utype, const f32 (&p) [4]) : name(name), utype(utype) { payload.InitWith<Array<f32, 4>>(p); }
        ShaderParameter(Str name, ShaderUniformType utype, Bytes p) : name(name), utype(utype) { payload.InitWith(p); }
        ShaderParameter(Str name, float x)                        : ShaderParameter(name, F_UNIT, { x, 0.f, 0.f, 0.f }) {}
        ShaderParameter(Str name, const Math::fVector2& v2s)      : ShaderParameter(name, FV2,    { v2s.x, v2s.y, 0.f, 0.f }) {}
        ShaderParameter(Str name, const Math::fVector3& v3s)      : ShaderParameter(name, FV3,    { v3s.x, v3s.y, v3s.z, 0.f }) {}
        ShaderParameter(Str name, const Math::fVector4& v4s)      : ShaderParameter(name, FV4,    { v4s.x, v4s.y, v4s.z, v4s.w }) {}
        ShaderParameter(Str name, Span<const float> xs)           : ShaderParameter(name, F_ARRAY, xs.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::fVector2> v2s) : ShaderParameter(name, FV2,    v2s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::fVector3> v3s) : ShaderParameter(name, FV3,    v3s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::fVector4> v4s) : ShaderParameter(name, FV4,    v4s.AsBytes()) {}
        ShaderParameter(Str name, int x)                          : ShaderParameter(name, I_UNIT, { (u32)x, 0u, 0u, 0u }) {}
        ShaderParameter(Str name, const Math::iVector2& v2s)      : ShaderParameter(name, IV2,    { (u32)v2s.x, (u32)v2s.y, 0u, 0u }) {}
        ShaderParameter(Str name, const Math::iVector3& v3s)      : ShaderParameter(name, IV3,    { (u32)v3s.x, (u32)v3s.y, (u32)v3s.z, 0u }) {}
        ShaderParameter(Str name, const Math::iVector4& v4s)      : ShaderParameter(name, IV4,    { (u32)v4s.x, (u32)v4s.y, (u32)v4s.z, (u32)v4s.w }) {}
        ShaderParameter(Str name, Span<const int> xs)             : ShaderParameter(name, I_ARRAY, xs.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::iVector2> v2s) : ShaderParameter(name, IV2,    v2s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::iVector3> v3s) : ShaderParameter(name, IV3,    v3s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::iVector4> v4s) : ShaderParameter(name, IV4,    v4s.AsBytes()) {}
        ShaderParameter(Str name, uint x)                         : ShaderParameter(name, I_UNIT, { x, 0u, 0u, 0u }) {}
        ShaderParameter(Str name, const Math::uVector2& v2s)      : ShaderParameter(name, IV2,    { v2s.x, v2s.y, 0u, 0u }) {}
        ShaderParameter(Str name, const Math::uVector3& v3s)      : ShaderParameter(name, IV3,    { v3s.x, v3s.y, v3s.z, 0u }) {}
        ShaderParameter(Str name, const Math::uVector4& v4s)      : ShaderParameter(name, IV4,    { v4s.x, v4s.y, v4s.z, v4s.w }) {}
        ShaderParameter(Str name, Span<const uint> xs)            : ShaderParameter(name, U_ARRAY, xs.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::uVector2> v2s) : ShaderParameter(name, UV2,    v2s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::uVector3> v3s) : ShaderParameter(name, UV3,    v3s.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::uVector4> v4s) : ShaderParameter(name, UV4,    v4s.AsBytes()) {}
        ShaderParameter(Str name, const Math::fColor3& color3);
        ShaderParameter(Str name, const Math::fColor&  color);
        ShaderParameter(Str name, Span<const Math::fColor3> color3s);
        ShaderParameter(Str name, Span<const Math::fColor>  colors);
        ShaderParameter(Str name, const Texture& tex);
        ShaderParameter(Str name, const Math::Matrix2x2& mat)       : ShaderParameter(name, FMAT_2X2, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix2x3& mat)       : ShaderParameter(name, FMAT_2X3, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix2x4& mat)       : ShaderParameter(name, FMAT_2X4, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix3x2& mat)       : ShaderParameter(name, FMAT_3X2, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix3x3& mat)       : ShaderParameter(name, FMAT_3X3, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix3x4& mat)       : ShaderParameter(name, FMAT_3X4, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix4x2& mat)       : ShaderParameter(name, FMAT_4X2, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix4x3& mat)       : ShaderParameter(name, FMAT_4X3, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, const Math::Matrix4x4& mat)       : ShaderParameter(name, FMAT_4X4, Bytes::BytesOf(mat)) {}
        ShaderParameter(Str name, Span<const Math::Matrix2x2> mats) : ShaderParameter(name, FMAT_2X2, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix2x3> mats) : ShaderParameter(name, FMAT_2X3, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix2x4> mats) : ShaderParameter(name, FMAT_2X4, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix3x2> mats) : ShaderParameter(name, FMAT_3X2, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix3x3> mats) : ShaderParameter(name, FMAT_3X3, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix3x4> mats) : ShaderParameter(name, FMAT_3X4, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix4x2> mats) : ShaderParameter(name, FMAT_4X2, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix4x3> mats) : ShaderParameter(name, FMAT_4X3, mats.AsBytes()) {}
        ShaderParameter(Str name, Span<const Math::Matrix4x4> mats) : ShaderParameter(name, FMAT_4X4, mats.AsBytes()) {}
    };

    struct ShaderArgs {
        // packed in: [utype] [byteCount (8 bytes)] [payload (arbitrary)] [unifName (arbitrary)] [NULL]
        Vec<u8> rawBytes;

        ShaderArgs() {}
        ShaderArgs(IList<ShaderParameter> p);

        ShaderArgs& Then(const ShaderParameter& val);
    };
}
