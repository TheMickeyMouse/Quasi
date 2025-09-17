#pragma once
#include "GLs/FrameBuffer.h"
#include "GLs/RenderBuffer.h"
#include "GLs/Shader.h"
#include "GLs/Texture.h"

namespace Quasi::Graphics {
    class RenderData;

    struct Bloom {
        FrameBuffer screenTex;
        RenderBuffer depthBuffer;
        Texture2D downsample;
        ShaderProgram highPass, downsampler, upsampler, addBack;
        Math::uv2 screenDim;

        float threshold = 1.0f, kneeOff = 0.3f, intensity = 0.2f;

        Bloom(const Math::uv2& screenDim);

        void SetToRenderTarget();
        void ApplyEffect();
    };
} // Quasi
