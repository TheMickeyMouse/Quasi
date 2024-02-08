//#shader vertex
#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float rtype;
out vec4 v_color;
out vec2 v_TexCoord;
flat out int v_rtype;
uniform mat4 u_projection;
uniform mat4 u_view;
void main() {
   gl_Position = u_projection * u_view * position;
   v_rtype = int(rtype);
   v_TexCoord = texCoord;
   v_color = color;
}
//#shader fragment
//#version 330 core
//layout(location = 0) out vec4 color;
//in vec4 v_color;
//in vec2 v_TexCoord;
//flat in int v_rtype;
//uniform sampler2D u_font;
//void main() {
//   color = v_color;
//   if (v_rtype == 1) {
//       float alpha = texture(u_font, v_TexCoord).r;
//       alpha = smoothstep(0.45, 0.55, alpha);
//       color.a = alpha;
//   }
//}