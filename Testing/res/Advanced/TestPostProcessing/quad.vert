#version 330

out vec2 TexCoords;
void main() {
    gl_Position = vec4(vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3))[gl_VertexID], 0, 1);
    TexCoords = gl_Position.xy * 0.5 + 0.5;
}