#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 auv;

out vec2 uv;
out vec3 fragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main() {
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    fragPos = (aPos * 2.0 + vec3(1.0)) / 2.0;
    uv = auv;
}
