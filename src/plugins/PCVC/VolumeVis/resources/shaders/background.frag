#version 430

#define M_PI 3.14159265358979323846

uniform float aspect;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Draw background pattern.
    // --------------------------------------------------------------------------------
    vec2 gridCoords = texCoords * vec2(100.0, 100.0 / aspect);

    float gridX = abs(fract(gridCoords.x) - 0.5);
    float gridY = abs(fract(gridCoords.y) - 0.5);

    float lineThickness = 0.05;
    float grid = step(lineThickness, gridX) * step(lineThickness, gridY);

    fragColor = mix(vec4(0.9, 0.9, 0.9, 1.0), vec4(0.7, 0.7, 0.7, 1.0), grid);
}
