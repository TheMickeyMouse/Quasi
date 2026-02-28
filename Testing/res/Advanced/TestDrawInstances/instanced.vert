#version 330 core

#define INSTANCE_NUM 27

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

out vec3 vNormal;
out vec3 vColor;

uniform mat4 u_projection, u_view;
uniform mat4 models[INSTANCE_NUM];
uniform mat3 normMat[INSTANCE_NUM];
uniform vec3 colors[INSTANCE_NUM];

void main() {
    gl_Position = u_projection * u_view * models[gl_InstanceID] * position;
    vNormal = normalize(vec3(normMat[gl_InstanceID] * normal));
    vColor = colors[gl_InstanceID];
}
