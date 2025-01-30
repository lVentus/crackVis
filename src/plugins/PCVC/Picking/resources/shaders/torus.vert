#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

uniform mat4 modelMx;
uniform mat4 viewMx;
uniform mat4 projMx;

out vec2 texCoords;
out vec3 normal;

void main() {

    normal = normalize(inNormal);

    texCoords = inTexCoord;
    gl_Position = projMx * viewMx *  modelMx * vec4(inPosition, 1.0);
}
