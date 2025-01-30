#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform sampler2D tex;
uniform isampler2D idTex;
uniform sampler2D normalTex;
uniform sampler2D depthTex;
uniform int showMode;

uniform float lightLong;
uniform float lightLat;
uniform float lightDist;
uniform float lightFoV;

uniform mat4 projMx;
uniform mat4 viewMx;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

vec4 getColorFromID(int id) {
    switch (id) {
        case 1:
            return vec4(1.0, 0.0, 0.0, 1.4);
        case 2:
            return vec4(0.0, 1.0, 0.0, 1.4);
        case 3:
            return vec4(0.0, 0.0, 1.0, 1.4);
        case 4:
            return vec4(1.0, 1.0, 0.0, 1.4);
        default:
            return vec4(0.0, 0.0, 0.0, 1.4);
    };
};

void main() {
    vec4 color = vec4(texCoords, 0.0, 1.0);

    float Ia = 1.0;
    float Iin = 1.0;
    float kamb = 0.2;
    float kdiff = 0.8;
    float kspec = 0.0;

    float Iamb = kamb * Ia;
    float Idiff = 0.0;
    float Ispec = 0.0;

    if(showMode == 1)
        fragColor = getColorFromID(texture(idTex, texCoords).r);
    else if(showMode == 2)
        fragColor = texture(normalTex, texCoords);
    else if(showMode == 3)
        fragColor = 100 * (vec4(1.0) - texture(depthTex, texCoords).rrrr);
    else{
        vec3 albedo = texture(tex, texCoords).rgb;
        vec3 normal = normalize(texture(normalTex, texCoords).rgb);
        float depth = texture(depthTex, texCoords).r;
        

        vec4 screenPos = vec4(texCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    
        vec4 worldPos = inverse(projMx * viewMx) * screenPos;
        worldPos /= worldPos.w;

        vec3 lightDir;
        lightDir.x = lightDist * cos(lightLat) * cos(lightLong);
        lightDir.y = lightDist * sin(lightLat);
        lightDir.z = lightDist * cos(lightLat) * sin(lightLong);
        lightDir = normalize(lightDir - worldPos.xyz);

        vec3 ambient = vec3(0.6) * albedo;
        vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(1.0) * albedo;

        fragColor = vec4(ambient + diffuse, 3.0);
        //fragColor = texture(tex, texCoords);
    };

}
