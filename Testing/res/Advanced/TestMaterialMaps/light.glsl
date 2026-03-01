// #shader vertex
#version 330
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
out vec4 vColor;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform vec3 lightPos;
void main() {
   gl_Position = u_projection * u_view * vec4(position.xyz + lightPos, position.w);
   vColor = color;
}
// #shader fragment
#version 330
layout(location = 0) out vec4 glColor;
in vec4 vColor;
void main() {
    glColor = vColor;
}