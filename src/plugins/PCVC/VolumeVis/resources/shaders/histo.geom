#version 430

uniform mat4 orthoProjMx;
uniform float binStepHalf;

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

void main() {
    vec4 v = gl_in[0].gl_Position;
    // --------------------------------------------------------------------------------
    //  TODO: Expand the single point v to a quad, representing the bin.
    // --------------------------------------------------------------------------------

    vec4 bottomLeft = vec4(v.x - binStepHalf, 0.0, 0.0, 1.0);
    vec4 topLeft = vec4(v.x - binStepHalf, v.y, 0.0, 1.0);
    vec4 bottomRight = vec4(v.x + binStepHalf, 0.0, 0.0, 1.0);
    vec4 topRight = vec4(v.x + binStepHalf, v.y, 0.0, 1.0);

    gl_Position = orthoProjMx * bottomLeft;
    EmitVertex();

    gl_Position = orthoProjMx * bottomRight;
    EmitVertex();

    gl_Position = orthoProjMx * topLeft;
    EmitVertex();

    gl_Position = orthoProjMx * topRight;
    EmitVertex();

    EndPrimitive();
}
