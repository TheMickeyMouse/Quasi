// #shader vertex
#version 330
out vec2 pos;
void main() {
   gl_Position = vec4(vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3))[gl_VertexID], 0, 1);
   pos = (gl_Position.xy + 1) * vec2(8, 4.5);
}
// #shader fragment
#version 330 
in vec2 pos; 
layout (location = 0) out vec4 glColor; 
uniform float radius; 
uniform float height; 
uniform vec2 rectCenter, rectSize; 
uniform vec3 lightSource;
uniform sampler2D glassFloor;
uniform float wave[6];
uniform vec2 waveOrigin[6];

float pow4(float x) {
   x *= x;
   x *= x;
   return x;
}

void main() {
   vec2 rPos = pos - rectCenter;
   vec2 d = abs(rPos) - rectSize;
   vec2 p = max(d, 0.0) * sign(rPos);
   float lensq = dot(p, p);
   if (lensq >= radius * radius) {
       discard;
   }
   vec3 normal  = vec3(p / radius, sqrt(1 - lensq / (radius * radius)));
   vec3 hit     = normal * radius;
   vec3 rDir    = refract(vec3(0, 0, -1), normal, 0.667);
   vec3 hit2    = hit - 2 * radius * rDir * dot(rDir, normal);
   vec3 n2      = -hit2 / radius;
   vec3 xDir    = refract(rDir, n2, 1.5);
   vec2 surface = hit2.xy - xDir.xy * ((height + hit2.z) / xDir.z) + pos - p;
   vec3 result  = texture(glassFloor, surface / vec2(16, 9)).rgb;
   result *= 1 + pow4((dot(normalize(lightSource - hit - vec3(pos - p, 0.0)), normal)));
   for (int i = 0; i < 6; ++i) {
      float lrel = length(surface - waveOrigin[i]);
      result += 0.33 * max(0, 0.6 - lrel) * smoothstep(0.1, 0, abs(lrel - wave[i]));
   }
   glColor = vec4(result, 1.0);
}