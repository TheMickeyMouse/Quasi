#pragma once
#include "GLs/FrameBuffer.h"
#include "GLs/RenderBuffer.h"
#include "GLs/Shader.h"
#include "GLs/Texture.h"

namespace Quasi::Graphics {
    struct Bloom {
        FrameBuffer screenTex, blitter;
        RenderBuffer depthBuffer;
        Texture downsample, upsample;
        ShaderProgram highPass, downsampler, upsampler, addBack;
        Math::uv2 screenDim;

        float threshold = 1.0f, kneeOff = 0.3f, intensity = 0.2f;

        Bloom(const Math::uv2& screenDim);

        void PreRender();
        void ApplyEffect();
    };
} // Quasi
