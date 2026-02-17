#include "Bloom.h"

#include "glp.h"
#include "GraphicsDevice.h"
#include "RenderData.h"
#include "GLs/GLDebug.h"
#include "GLs/Render.h"

namespace Quasi::Graphics {
    Bloom::Bloom(const Math::iv2& screenDim) : screenDim(screenDim) {
        screenTex = FrameBuffer::New();
        depthBuffer = RenderBuffer::New(
            TextureIFormat::DEPTH, screenDim
        );
        downsample = Texture2D::New(nullptr, screenDim, {
            .format = TextureFormat::RGBA, .internalformat = TextureIFormat::RGBA_32F, .type = GLTypeID::FLOAT,
        });
        downsample.Bind();
        downsample.GenerateEmptyMipmaps(6);
        downsample.SetMaxMip(6);
        downsample.SetMinifySample(TextureSample::LINEAR_NEAREST_MIP);

        screenTex.Bind();
        screenTex.Attach(depthBuffer, AttachmentType::DEPTH);
        screenTex.Attach(downsample);
        screenTex.Unbind();

        highPass = ShaderProgram::NewCompute(
            "#version 430 core\n"
            "layout (local_size_x = 1, local_size_y = 1) in;\n"
            "layout (rgba32f, binding = 0) coherent uniform writeonly image2D imgOutput;\n"
            "layout (rgba32f, binding = 1) coherent uniform readonly  image2D imgInput;\n"
            "layout (location = 0) uniform float threshold;\n"
            "layout (location = 1) uniform float kneeOff;\n"
            "void main() {\n"
            "   ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n"
            "   vec4 input = imageLoad(imgInput, uv * 2); \n"
            "   float brightness = dot(input.rgb, vec3(0.2126, 0.7152, 0.0722)); \n"
            "   float transparency = smoothstep(threshold - kneeOff, threshold, brightness);"
            "   imageStore(imgOutput, uv, vec4(input.rgb * transparency, 1.0));\n"
            "}"
        );
        downsampler = ShaderProgram::NewCompute(
            "#version 430 core\n"
            "layout (local_size_x = 1, local_size_y = 1) in;\n"
            "layout (rgba32f, binding = 0) uniform image2D imgOutput;\n"
            "layout (location = 0) uniform sampler2D imgInput;\n"
            "layout (location = 1) uniform float lod;\n"
            "void main() {\n"
            "   vec2 s = 1.0 / vec2(gl_NumWorkGroups.xy); \n"
            "   vec2 uv = s * (gl_GlobalInvocationID.xy + 0.5); \n"
            "   vec4 A = textureLod(imgInput, uv + s * vec2(-1.0, -1.0), lod);"
            "   vec4 B = textureLod(imgInput, uv + s * vec2( 0.0, -1.0), lod);"
            "   vec4 C = textureLod(imgInput, uv + s * vec2( 1.0, -1.0), lod);"
            "   vec4 D = textureLod(imgInput, uv + s * vec2(-0.5, -0.5), lod);"
            "   vec4 E = textureLod(imgInput, uv + s * vec2( 0.5, -0.5), lod);"
            "   vec4 F = textureLod(imgInput, uv + s * vec2(-1.0,  0.0), lod);"
            "   vec4 G = textureLod(imgInput, uv                       , lod);"
            "   vec4 H = textureLod(imgInput, uv + s * vec2( 1.0,  0.0), lod);"
            "   vec4 I = textureLod(imgInput, uv + s * vec2(-0.5,  0.5), lod);"
            "   vec4 J = textureLod(imgInput, uv + s * vec2( 0.5,  0.5), lod);"
            "   vec4 K = textureLod(imgInput, uv + s * vec2(-1.0,  1.0), lod);"
            "   vec4 L = textureLod(imgInput, uv + s * vec2( 0.0,  1.0), lod);"
            "   vec4 M = textureLod(imgInput, uv + s * vec2( 1.0,  1.0), lod);"
            "   vec4 result = (D + E + G + I + J) * 0.125 + (B + F + H + L) * 0.0625 + (A + C + K + M) * 0.03125;"
            "   imageStore(imgOutput, ivec2(gl_GlobalInvocationID.xy), result); \n"
            "}"
            // "#version 430 core\n"
            // "layout (local_size_x = 1, local_size_y = 1) in;\n"
            // "layout (rgba32f, binding = 0) uniform image2D imgOutput;\n"
            // "layout (location = 0) uniform sampler2D imgInput;\n"
            // "layout (location = 1) uniform float lod;\n"
            // "void main() {\n"
            // "   vec2 s = 1.0 / vec2(gl_NumWorkGroups.xy); \n"
            // "   vec2 uv = s * vec2(gl_GlobalInvocationID.xy); \n"
            // "   vec4 A = texture(imgInput, uv + s * vec2(-0.5, -0.5));"
            // "   vec4 B = texture(imgInput, uv + s * vec2( 0.5, -0.5));"
            // "   vec4 C = texture(imgInput, uv                       );"
            // "   vec4 D = texture(imgInput, uv + s * vec2(-0.5,  0.5));"
            // "   vec4 E = texture(imgInput, uv + s * vec2( 0.5,  0.5));"
            // "   vec4 result = (A + B + D + E) * 0.125 + C * 0.5;"
            // "   imageStore(imgOutput, ivec2(gl_GlobalInvocationID.xy), result); \n"
            // "}"
            // "#version 430 core\n"
            // "layout (local_size_x = 1, local_size_y = 1) in;\n"
            // "layout (rgba32f, binding = 0) coherent uniform writeonly image2D imgOutput;\n"
            // "layout (rgba32f, binding = 1) coherent uniform readonly  image2D imgInput;\n"
            // "void main() {\n"
            // "   ivec2 imgSize = imageSize(imgInput); "
            // "   ivec2 uv = 2 * ivec2(gl_GlobalInvocationID.xy) + (imgSize & 1) * ivec2(greaterThan(gl_GlobalInvocationID.xy, gl_NumWorkGroups.xy >> 1)); \n"
            // "   vec3 A = imageLoad(imgInput, uv + ivec2( 0,  2)).rgb;"
            // "   vec3 B = imageLoad(imgInput, uv + ivec2( 1,  2)).rgb;"
            // "   vec3 C = imageLoad(imgInput, uv + ivec2(-1,  1)).rgb;"
            // "   vec3 D = imageLoad(imgInput, uv + ivec2( 0,  1)).rgb;"
            // "   vec3 E = imageLoad(imgInput, uv + ivec2( 1,  1)).rgb;"
            // "   vec3 F = imageLoad(imgInput, uv + ivec2( 2,  1)).rgb;"
            // "   vec3 G = imageLoad(imgInput, uv + ivec2(-1,  0)).rgb;"
            // "   vec3 H = imageLoad(imgInput, uv + ivec2( 0,  0)).rgb;"
            // "   vec3 I = imageLoad(imgInput, uv + ivec2( 1,  0)).rgb;"
            // "   vec3 J = imageLoad(imgInput, uv + ivec2( 2,  0)).rgb;"
            // "   vec3 K = imageLoad(imgInput, uv + ivec2( 0, -1)).rgb;"
            // "   vec3 L = imageLoad(imgInput, uv + ivec2( 1, -1)).rgb;"
            // "   vec3 result = (D + E + H + I) * 0.125 + (A + B + C + F + G + J + K + L) * 0.0625;"
            // "   imageStore(imgOutput, ivec2(gl_GlobalInvocationID.xy), vec4(result, 1.0)); \n"
            // "}"
        );

        upsampler = ShaderProgram::NewCompute(
            // "#version 430 core\n"
            // "layout (local_size_x = 1, local_size_y = 1) in;\n"
            // "layout (rgba32f, binding = 0) uniform image2D imgOutput;\n"
            // "layout (rgba32f, binding = 1) uniform image2D imgInput;\n"
            // "void main() {\n"
            // "   ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n"
            // "   vec2 uvf = (vec2(uv) + 0.5) * imageSize(imgInput) / vec2(gl_NumWorkGroups.xy);"
            // "   vec2 majorUV = round(uvf);"
            // "   vec2 minorUV = uvf - majorUV;"
            // "   ivec4 st = ivec4(ivec2(majorUV), ivec2(majorUV) - 1); "
            // "   vec3 A = imageLoad(imgInput, st.xy).rgb;"
            // "   vec3 B = imageLoad(imgInput, st.xw).rgb;"
            // "   vec3 C = imageLoad(imgInput, st.zy).rgb;"
            // "   vec3 D = imageLoad(imgInput, st.zw).rgb;"
            // "   vec4 weights = vec4(0.5 + minorUV, 0.5 - minorUV); "
            // "   vec3 linear = weights.y * (weights.x * A + weights.z * C) + weights.w * (weights.x * B + weights.z * D);"
            // "   vec3 result = imageLoad(imgOutput, uv).rgb + 0.9 * linear;"
            // "   imageStore(imgOutput, uv, vec4(result, 1.0)); \n"
            // "}"
            "#version 430 core\n"
            "layout (local_size_x = 1, local_size_y = 1) in;\n"
            "layout (rgba32f, binding = 0) uniform image2D imgOutput;\n"
            "layout (location = 0) uniform sampler2D imgInput;\n"
            "layout (location = 1) uniform float lod;"
            "void main() {\n"
            "   vec2 s = 1.0 / vec2(gl_NumWorkGroups.xy); \n"
            "   ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n"
            "   vec2 uvf = uv * s;"
            "   vec3 upsampled = (textureLod(imgInput, uvf,                lod).rgb +"
            "                     textureLod(imgInput, uvf + vec2(s.x, 0), lod).rgb +"
            "                     textureLod(imgInput, uvf + vec2(0, s.y), lod).rgb +"
            "                     textureLod(imgInput, uvf + s,            lod).rgb) * 0.25;"
            "   vec3 result = upsampled + imageLoad(imgOutput, uv).rgb;"
            "   imageStore(imgOutput, uv, vec4(result, 1.0)); \n"
            "}"
        );

        addBack = ShaderProgram::NewCompute(
            "#version 430 core\n"
            "layout (local_size_x = 1, local_size_y = 1) in;\n"
            "layout (rgba32f, binding = 0) uniform image2D imgOutput;\n"
            "layout (rgba32f, binding = 1) uniform image2D currentLod;\n"
            "layout (location = 0) uniform float intensity;\n"
            "void main() {\n"
            "   ivec2 uv = ivec2(gl_GlobalInvocationID.xy); \n"
            "   vec3 result = imageLoad(imgOutput, uv).rgb + intensity * imageLoad(currentLod, uv / 2).rgb; \n"
            "   float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722)); \n"
            "   result *= (4 / (4 + brightness));"
            "   imageStore(imgOutput, uv, vec4(result, 1.0)); \n"
            "}"
        );
    }

    void Bloom::SetToRenderTarget() {
        screenTex.BindDrawDest();
        GL::Viewport(0, 0, screenDim.x, screenDim.y);
    }

    void Bloom::ApplyEffect() {
        constexpr u32 SLOT_OUTPUT = 0, SLOT_INPUT = 1,
                      UNIF_THRESHOLD = 0,
                      UNIF_KNEEOFF = 1,
                      UNIF_INPUT_TEXTURE = 0,
                      UNIF_TEXTURE_LOD = 1,
                      UNIF_INTENSITY = 0;

        downsample.BindImageTexture(SLOT_INPUT, 0, Access::READ);
        downsample.BindImageTexture(SLOT_OUTPUT, 1, Access::WRITE);
        highPass.Bind();
        GL::Uniform1f(UNIF_THRESHOLD, threshold);
        GL::Uniform1f(UNIF_KNEEOFF, kneeOff);
        highPass.ExecuteCompute(screenDim.x / 2, screenDim.y / 2);

        Render::MemoryBarrier(MemBarrier::SHADER_IMAGE_ACCESS);

        downsampler.Bind();
        downsample.Activate(15);
        GL::Uniform1i(UNIF_INPUT_TEXTURE, 15);
        for (int i = 2; i <= 6; ++i) {
            downsample.BindImageTexture(SLOT_OUTPUT, i, Access::WRITE);
            GL::Uniform1f(UNIF_TEXTURE_LOD, (float)(i - 1));
            downsampler.ExecuteCompute(screenDim.x >> i, screenDim.y >> i);

            Render::MemoryBarrier(MemBarrier::SHADER_IMAGE_ACCESS);
        }

        upsampler.Bind();
        GL::Uniform1i(UNIF_INPUT_TEXTURE, 15);
        for (int i = 6; i --> 1;) {
            downsample.BindImageTexture(SLOT_OUTPUT, i, Access::WRITE);
            GL::Uniform1f(UNIF_TEXTURE_LOD, (float)(i + 1));
            // downsample.BindImageTexture(SLOT_INPUT, i + 1);
            upsampler.ExecuteCompute(screenDim.x >> i, screenDim.y >> i);

            Render::MemoryBarrier(MemBarrier::SHADER_IMAGE_ACCESS);
        }

        addBack.Bind();
        GL::Uniform1f(UNIF_INTENSITY, intensity);
        downsample.BindImageTexture(SLOT_OUTPUT, 0, Access::WRITE);
        downsample.BindImageTexture(SLOT_INPUT, 1, Access::READ);
        addBack.ExecuteCompute(screenDim.x, screenDim.y);

        Render::MemoryBarrier(MemBarrier::SHADER_IMAGE_ACCESS);

        const Math::iv2 actualScreenDim = GraphicsDevice::GetDeviceInstance().GetWindowSize();
        screenTex.BlitToScreen({ 0, (screenDim).As<int>() }, { 0, actualScreenDim });
        screenTex.Unbind();

        GL::Viewport(0, 0, actualScreenDim.x, actualScreenDim.y);
    }
}
