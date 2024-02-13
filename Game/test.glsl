#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec2 off = 1.0 / vec2(textureSize(screenTexture));
    vec3 total = vec3(0.0);
    for (int i = -4; i < 5; ++i) {
        float multiplier = float(1 << ((i & 1) + 2 * int(i == 0))) / 16.0;
        vec2 offset = off * vec2(float((i + 4) % 3), float((i + 4) / 3));
        total += multiplier * texture(screenTexture, TexCoords - off + offset);
    }
    FragColor = vec4(total, 1.0);
}