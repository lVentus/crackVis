#version 430

// --------------------------------------------------------------------------------
//  TODO: Complete this shader!
// --------------------------------------------------------------------------------

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

layout(points) in;
layout(triangle_strip, max_vertices=36) out;

out vec3 normal;
out vec2 texCoords;

const vec3 vertices[14] = vec3[](
    vec3(-0.5, -0.5, -0.5), // (left bottom back)
    vec3( 0.5, -0.5, -0.5), // (right bottom back)
    vec3(-0.5,  0.5, -0.5), // (left top back)
    vec3( 0.5,  0.5, -0.5), // (right top back)
    vec3(-0.5, -0.5,  0.5), // (left bottom front)
    vec3( 0.5, -0.5,  0.5), // (right bottom front)
    vec3(-0.5,  0.5,  0.5), // (left top front)
    vec3( 0.5,  0.5,  0.5), // (right top front)
    vec3(-0.5, -0.5, -0.5), // (left bottom back) - left face
    vec3(-0.5,  0.5, -0.5), // (left top back) - left face
    vec3( 0.5, -0.5, -0.5), // (right bottom back) - right face
    vec3( 0.5,  0.5, -0.5), // (right top back) - right face
    vec3(-0.5, -0.5, -0.5), // (left bottom back) - back face
    vec3(-0.5,  0.5, -0.5)  // (left top back) - back face
);

const vec2 uvs[14] = vec2[](
    vec2(0.25, 0.25), // 0
    vec2(0.5,  0.25), // 1
    vec2(0.25, 1.0),  // 2
    vec2(0.5,  1.0),  // 3
    vec2(0.25, 0.5),  // 4
    vec2(0.5,  0.5),  // 5
    vec2(0.25, 0.75), // 6
    vec2(0.5,  0.75), // 7
    vec2(0.0,  0.5),  // 8
    vec2(0.0,  0.75), // 9
    vec2(0.75, 0.5),  // 10
    vec2(0.75, 0.75), // 11
    vec2(1.0,  0.5),  // 12
    vec2(1.0,  0.75)  // 13
);

const int faces[36] = int[](
    12, 13, 10,  10, 13, 11, 
    5, 7, 4,  4, 7, 6,      
    1, 5, 0,  0, 5, 4,       
    7, 3, 6,  6, 3, 2,       
    4, 6, 8,  8, 6, 9,      
    10, 11, 5,  5, 11, 7     
);

void main() {
    for (int face = 0; face < 6; ++face) {
        int i0 = faces[face * 6 + 0];
        int i1 = faces[face * 6 + 1];
        int i2 = faces[face * 6 + 2];
        
        vec3 v0 = vertices[i0];
        vec3 v1 = vertices[i1];
        vec3 v2 = vertices[i2];
        
        vec3 faceNormal = normalize(cross(v1 - v0, v2 - v0));
        
        for (int i = 0; i < 6; ++i) {
            int vertexIndex = faces[face * 6 + i];
            
            normal = normalize(faceNormal);
            texCoords = uvs[vertexIndex];

            gl_Position = projMx * viewMx * modelMx * vec4(vertices[vertexIndex], 1.0);

            EmitVertex();
        }
        EndPrimitive();
    }
}
