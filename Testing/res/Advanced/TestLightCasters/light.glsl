// #shader vertex
#version 330
layout(location = 0) in vec4 position;
out vec4 vColor;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform vec3 lightPos[8];
uniform vec4 colors[8];
void main() {
   gl_Position = u_projection * u_view * vec4(position.xyz + lightPos[gl_InstanceID], position.w);
   vColor = vec4(colors[gl_InstanceID].xyz * 2, 1.0);
}
// #shader fragment
#version 330
layout(location = 0) out vec4 glColor;
in vec4 vColor;
void main() {
    glColor = vColor;
}