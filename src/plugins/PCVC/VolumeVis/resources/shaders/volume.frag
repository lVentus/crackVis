#version 430

#define M_PI 3.14159265358979323846

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

uniform sampler3D volumeTex; //!< 3D texture handle
uniform sampler1D transferTex;

uniform mat4 invViewMx;     //!< inverse view matrix
uniform mat4 invViewProjMx; //!< inverse view-projection matrix

uniform vec3 volumeRes; //!< volume resolution
uniform vec3 volumeDim; //!< volume dimensions

uniform int viewMode; //<! rendering method: 0: line-of-sight, 1: mip, 2: isosurface, 3: volume
uniform bool showBox;
uniform bool useRandom;

uniform int maxSteps;   //!< maximum number of steps
uniform float stepSize; //!< step size
uniform float scale;    //!< scaling factor

uniform float isovalue; //!< value for iso surface

uniform vec3 ambient;  //!< ambient color
uniform vec3 diffuse;  //!< diffuse color
uniform vec3 specular; //!< specular color

uniform float k_amb;  //!< ambient factor
uniform float k_diff; //!< diffuse factor
uniform float k_spec; //!< specular factor
uniform float k_exp;  //!< specular exponent

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

struct Ray {
    vec3 o; // origin of the ray
    vec3 d; // direction of the ray
};

/**
 * Test for an intersection with the bounding box.
 * @param r             The ray to use for the intersection test
 * @param boxmin        The minimum point (corner) of the box
 * @param boxmax        The maximum point (corner) of the box
 * @param tnear[out]    The distance to the closest plane intersection
 * @param tfar[out]     The distance to the farthest plane intersection
 */
bool intersectBox(Ray r, vec3 boxmin, vec3 boxmax, out float tnear, out float tfar) {
    // --------------------------------------------------------------------------------
    //  TODO: Calculate intersection between ray and box.
    // --------------------------------------------------------------------------------
    vec3 t0 = (boxmin - r.o) / r.d;
    vec3 t1 = (boxmax - r.o) / r.d;

    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    tnear = max(max(tmin.x, tmin.y), tmin.z);
    tfar = min(min(tmax.x, tmax.y), tmax.z);

    return tnear <= tfar && tfar >= 0.0;
}

/**
 * Test if the given position is near a box edge.
 * @param pos           The position to test against
 */
bool isBoxEdge(vec3 pos, float edgeThickness) {
    // --------------------------------------------------------------------------------
    //  TODO: Check if the position is near an edge of the volume cube.
    // --------------------------------------------------------------------------------
    bool closeToX = abs(pos.x - 0.5 * volumeDim.x) < edgeThickness || abs(pos.x + 0.5 * volumeDim.x) < edgeThickness;
    bool closeToY = abs(pos.y - 0.5 * volumeDim.y) < edgeThickness || abs(pos.y + 0.5 * volumeDim.y) < edgeThickness;
    bool closeToZ = abs(pos.z - 0.5 * volumeDim.z) < edgeThickness || abs(pos.z + 0.5 * volumeDim.z) < edgeThickness;

    int closeCount = int(closeToX) + int(closeToY) + int(closeToZ);
    return closeCount >= 2;
}

/**
 * Map world coordinates to texture coordinates.
 * @param pos           The world coordinates to transform to texture coordinates
 */
vec3 mapTexCoords(vec3 pos) {
    // --------------------------------------------------------------------------------
    //  TODO: Map world pos to texture coordinates for 3D volume texture.
    // --------------------------------------------------------------------------------
    return (pos / (volumeDim * scale)) * 0.5 + 0.5;
}

/**
 * Calculate normals based on the volume gradient.
 */
vec3 calcNormal(vec3 pos) {
    // --------------------------------------------------------------------------------
    //  TODO: Calculate normals based on volume gradient.
    // --------------------------------------------------------------------------------
    float dx = textureOffset(volumeTex, pos, ivec3(1, 0, 0)).r - 
               textureOffset(volumeTex, pos, ivec3(-1, 0, 0)).r;
    float dy = textureOffset(volumeTex, pos, ivec3(0, 1, 0)).r - 
               textureOffset(volumeTex, pos, ivec3(0, -1, 0)).r;
    float dz = textureOffset(volumeTex, pos, ivec3(0, 0, 1)).r - 
               textureOffset(volumeTex, pos, ivec3(0, 0, -1)).r;

    return normalize(vec3(dx, dy, dz)); 
}

/**
 * Calculate the correct pixel color using the Blinn-Phong shading model.
 * @param n             The normal at this pixel
 * @param l             The direction vector towards the light
 * @param v             The direction vector towards the viewer
 */
vec3 blinnPhong(vec3 n, vec3 l, vec3 v) {
    vec3 color = vec3(0.0);
    // --------------------------------------------------------------------------------
    //  TODO: Calculate correct Blinn-Phong shading.
    // --------------------------------------------------------------------------------
    
    vec3 h = normalize(l + v);
    float diff = max(dot(n, l), 0.0);
    float spec = pow(max(dot(n, h), 0.0), k_exp);

    return k_amb * ambient + k_diff * diff * diffuse + k_spec * spec * specular;
}

/**
 * Pseudorandom function, returns a float value between 0.0 and 1.0.
 * @param seed          Positive integer that acts as a seed
 */
float random(uint seed) {
    // wang hash
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return float(seed) / 4294967296.0;
}

/**
 * The main function of the shader.
 */
void main() {
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    // --------------------------------------------------------------------------------
    //  TODO: Set up the ray and box. Do the intersection test and draw the box.
    // --------------------------------------------------------------------------------
    Ray ray;
    ray.o = (invViewMx * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec4 worldPoint = invViewProjMx * vec4(texCoords * 2.0 - 1.0, 1.0, 1.0);
    worldPoint /= worldPoint.w;
    ray.d = normalize(worldPoint.xyz - ray.o);

    float tnear, tfar;
    if (!intersectBox(ray, -0.5 * volumeDim, 0.5 * volumeDim, tnear, tfar)) {
        discard;
    }
    if (showBox == true && isBoxEdge(ray.o + tfar * ray.d, 0.005)) {
        color = vec4(0.0, 1.0, 1.0, 1.0);
    }
    // --------------------------------------------------------------------------------
    //  TODO: Draw the volume based on the current view mode.
    // --------------------------------------------------------------------------------
    switch (viewMode) {
        case 0: { // line-of-sight
            // --------------------------------------------------------------------------------
            //  TODO: Implement line of sight (LoS) rendering.
            // --------------------------------------------------------------------------------
            vec3 currentPoint = ray.o + tnear * ray.d;
            vec3 step = ray.d * stepSize;

            for (float t = tnear; t < tfar && t < tnear + maxSteps * stepSize; t += stepSize) {
                vec3 texCoord = mapTexCoords(currentPoint);
                float value = texture(volumeTex, texCoord).r * 0.1;

                color.rgb += vec3(value);
                color.a = 1.0;

                currentPoint += step;
            }
            break;
        }
        case 1: { // maximum-intesity projection
            // --------------------------------------------------------------------------------
            //  TODO: Implement maximum intensity projection (MIP) rendering.
            // --------------------------------------------------------------------------------
            vec3 currentPoint = ray.o + tnear * ray.d;
            vec3 step = ray.d * stepSize;
            float maxValue = 0.0;

            for (float t = tnear; t < tfar && t < tnear + maxSteps * stepSize; t += stepSize) {
                vec3 texCoord = mapTexCoords(currentPoint);
                float value = texture(volumeTex, texCoord).r;

                maxValue = max(maxValue, value);
                currentPoint += step;
            }

            color = vec4(maxValue,maxValue,maxValue, 1.0);
            break;
        }
        case 2: { // isosurface
            // --------------------------------------------------------------------------------
            //  TODO: Implement isosurface rendering.
            // --------------------------------------------------------------------------------
            vec3 currentPoint = ray.o + tnear * ray.d;
            vec3 step = ray.d * stepSize;

            float prevValue = texture(volumeTex, mapTexCoords(currentPoint)).r;
            currentPoint += step;

            for (float t = tnear; t < tfar; t += stepSize) {
                vec3 texCoord = mapTexCoords(currentPoint);
                float currentValue = texture(volumeTex, texCoord).r;

                if ((prevValue - isovalue) * (currentValue - isovalue) < 0.0) {
                    float delta = (currentValue - isovalue) / (currentValue - prevValue);
                    vec3 isoPoint = currentPoint - delta * step;

                    vec3 normal = calcNormal(mapTexCoords(isoPoint));

                    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
                    vec3 viewDir = normalize(-ray.d);

                    color.rgb = blinnPhong(-normal, lightDir, viewDir);
                    color.a = 1.0;

                    break;
                }
                prevValue = currentValue;
                currentPoint += step;
            }
            
            if(color.r <0.2 && !(isBoxEdge(ray.o + tnear * ray.d, 0.005) || isBoxEdge(ray.o + tfar * ray.d, 0.005))){
                color.a = 0.0;
            }
            break;
        }
        case 3: { // volume visualization with transfer function
            // --------------------------------------------------------------------------------
            //  TODO: Implement volume rendering.
            // --------------------------------------------------------------------------------
            break;
        }
        default: {
            color = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        }
    }
    
    if (showBox == true && isBoxEdge(ray.o + tnear * ray.d, 0.005)) {
        color = vec4(0.0, 1.0, 1.0, 1.0);
    }
    // --------------------------------------------------------------------------------
    //  TODO: Draw the box lines behind the volume, if the volume is transparent.
    // --------------------------------------------------------------------------------
    fragColor = color;
}
