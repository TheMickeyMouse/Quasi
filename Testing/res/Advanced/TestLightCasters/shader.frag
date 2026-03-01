#version 330

layout (location = 0) out vec4 glColor;

in vec3 vPosition;
in vec3 vNormal;
flat in int vMatId;

uniform float ambientStrength;
uniform vec3 viewPosition;
uniform float specularIntensity;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

#define MATERIAL_NUM 8
uniform Material materials[MATERIAL_NUM];
#define LIGHT_NUM 8
uniform vec3 lightLoc[LIGHT_NUM];
// data memory: (000 1111 2222 333)
// sunlight:    -------------------
// pointlight:  [.c] [.l] [.q] ----
// flashlight:  [.y] [.p] [.i] [.o]
uniform vec4 lightInfo[LIGHT_NUM];
uniform vec4 lightColor[LIGHT_NUM];
#define SUNLIGHT_ID 1
#define POINTLIGHT_ID 2
#define FLASHLIGHT_ID 3
uniform int  lightId[LIGHT_NUM];

vec3 Diffuse(vec3 color, vec3 norm, vec3 lightDir) {
    float diffuseStrength = max(dot(norm, lightDir), 0.0);
    return diffuseStrength * color;
}

vec3 Specular(vec3 color, float shininess, vec3 norm, vec3 lightDir) {
    vec3 viewDir = normalize(viewPosition - vPosition);
    vec3 halfway = normalize(viewDir + lightDir);
    float specularStrength = pow(max(dot(norm, halfway), 0.0), shininess) * specularIntensity;
    return specularStrength * color;
}

vec3 Phong(Material m, vec4 color, vec3 lightDir) {
    vec3 ambient = ambientStrength * m.ambient;

    vec3 norm = normalize(vNormal);
    vec3 diffuse = Diffuse(m.diffuse, norm, lightDir);
    vec3 specular = Specular(m.specular, m.shininess, norm, lightDir);

    return (ambient + diffuse + specular) * color.rgb * color.a;
}

vec3 SunLight(Material m, int l) {
    return Phong(m, lightColor[l], normalize(-lightLoc[l]));
}

vec3 PointLight(Material m, int l) {
    // light attenuation
    float dist = length(lightLoc[l] - vPosition);
    vec3  info = lightInfo[l].xyz;
    float att = 1.0 / (info.x + info.y * dist + info.z * dist * dist);

    return Phong(m, lightColor[l], normalize(lightLoc[l] - vPosition)) * att;
}

vec3 FlashLight(Material m, int l) {
    vec3 lightDir = normalize(lightLoc[l] - vPosition);
    vec4 info = lightInfo[l];
    vec3 dir = vec3(sin(info.x) * cos(info.y), sin(info.y), cos(info.x) * cos(info.y));
    float angle = dot(lightDir, dir);
    vec3 amb = ambientStrength * m.ambient * lightColor[l].rgb * lightColor[l].a;
    return smoothstep(cos(info.w), cos(info.z), angle) * (Phong(m, lightColor[l], lightDir) - amb) + amb;
}

void main() {
    Material m = materials[vMatId];

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < LIGHT_NUM; ++i) {
        switch (lightId[i]) {
            case 0:
                break;
            case SUNLIGHT_ID:
                lighting += SunLight(m, i);
                break;
            case POINTLIGHT_ID:
                lighting += PointLight(m, i);
                break;
            case FLASHLIGHT_ID:
                lighting += FlashLight(m, i);
                break;
            default:
                break;
        }
    }
    glColor = vec4(lighting, 1.0);
}