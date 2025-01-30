#version 430
   
in vec3 normal;         
in vec2 texCoords;    

uniform int pickId;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outId;
layout(location = 2) out vec3 outNormal;

void main() {
    float scale = 10.0; 
    float checker = mod(floor(texCoords.x * scale) + floor(texCoords.y * scale), 2.0);
    vec3 color = checker < 1.0 ? vec3(0.5, 0.5, 0.5) : vec3(0.0, 0.0, 0.0); 

    outColor = vec4(color, 1.0);
    outId = pickId;
    outNormal = normal;
}

