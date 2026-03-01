// #shader vertex
#version 330
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

out vec3 vNorm;
out vec4 vColor;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform vec3 locations[9];
uniform vec4 colors[9];
uniform float scale;

void main() {
   gl_Position = u_projection * u_view * vec4(position.xyz * scale + locations[gl_InstanceID], position.w);
   vNorm = normal;
   vColor = colors[gl_InstanceID];
}
// #shader fragment
#version 330
layout(location = 0) out vec4 glColor;

in vec3 vNorm;
in vec4 vColor;

void main() {
    glColor = vec4(vColor.rgb * (0.8 + 0.3 * dot(vNorm, vec3(0.36, 0.48, 0.8))), vColor.a);
}