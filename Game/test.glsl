#version 330 core
layout(location = 0) out vec4 color;
in vec4 v_color;
in vec2 v_texCoord;
flat in int v_isText;
uniform sampler2D u_font;
uniform float u_thickness;
uniform float u_softness;
uniform vec2  u_shadowOffset;
uniform float u_shadowSoftness;
uniform vec4  u_shadowColor;
void main(){
    color = v_color;
    if (v_isText == 1) {
       float alpha = texture(u_font, v_texCoord).r;
       vec4 shadow = u_shadowColor;
       vec2 shadowCoord = v_texCoord - u_shadowOffset / textureSize(u_font, 0);
       float shadowAlpha = texture(u_font, shadowCoord).r;
       shadow.a = shadow.a * smoothstep(0.5 - u_shadowSoftness, 0.5 + u_shadowSoftness, shadowAlpha);
       alpha = smoothstep(1.0 - u_thickness - u_softness, 1.0 - u_thickness + u_softness, alpha);
       color = mix(shadow, color, alpha);
    }
}