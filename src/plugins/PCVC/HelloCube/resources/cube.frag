#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

in  vec2 uv;
in vec3 fragPos; 
out vec4 fragColor;

uniform float showTexture;
uniform int patternFreq;
uniform sampler2D Tex;
uniform mat4 scale;

void main() {
    vec4 textureColor = texture(Tex, uv);
    vec4 proceduralColor = vec4(fragPos, 1.0);

    vec3 pos = (scale * vec4(fragPos, 1.0)).xyz;
    float patternX = sin(pos.x * patternFreq * 3.14159 * 2.0) * 0.9999;
    float patternY = sin(pos.y * patternFreq * 3.14159 * 2.0) * 0.9999;
    float patternZ = sin(pos.z * patternFreq * 3.14159 * 2.0) * 0.9999;

    float checkerPattern = mod(floor(patternX + 1.0) + floor(patternY + 1.0) + floor(patternZ + 1.0), 2.0);

    proceduralColor = mix(vec4(0.0), proceduralColor, checkerPattern);

    fragColor = mix(proceduralColor, textureColor, showTexture);
}
