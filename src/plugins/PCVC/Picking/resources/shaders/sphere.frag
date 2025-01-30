#version 430

in vec3 normal;          
in vec2 texCoords;       

uniform sampler2D tex;
uniform int pickId;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outId;
layout(location = 2) out vec3 outNormal;

void main() {
    vec3 textureColor = texture(tex, texCoords).rgb;
    
    outColor = vec4(textureColor, 1.0);
    outId = pickId;
    outNormal = normalize(normal);
}
