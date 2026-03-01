#version 330 core

layout (location = 0) out vec4 glColor;

in vec3 vPosition;
in vec3 vNormal;
in vec4 vColor;

uniform float ambientStrength;

void main() {
    float diffuse = max(-dot(normalize(vNormal), normalize(vPosition)), 0.0);
    glColor = vec4(vColor.rgb * (ambientStrength + diffuse), vColor.a);
}