#version 430
   
in vec3 normal;         
in vec2 texCoords;    

uniform int pickId;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outId;
layout(location = 2) out vec3 outNormal;

void main() {
    outColor = texture(albedoMap, texCoords);
    outId = pickId;

    vec3 T = normalize(normalize(normal) * cross(normal, vec3(0.0, 0.0, 1.0)));
    vec3 B = cross(normal, T);
    mat3 TBN = mat3(T, B, normal);

    vec3 tangentNormal = texture(normalMap, texCoords).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    outNormal = normalize(TBN * tangentNormal);

}

