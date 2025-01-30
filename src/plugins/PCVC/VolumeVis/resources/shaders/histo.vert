#version 430

uniform float maxBinValue; //!< maximum value of all bins
uniform bool logPlot;      //!< whether to use a log scale

layout(location = 0) in vec2 in_position;

void main() {
    // --------------------------------------------------------------------------------
    //  TODO: Set information for the histogram bar.
    // --------------------------------------------------------------------------------
    float x = in_position.x;
    float y = in_position.y;

    if (logPlot) {
        y = log2(y + 1.0) / log2(maxBinValue + 1.0);
    } else {
        y /= maxBinValue;
    }

    gl_Position = vec4(x, y , 0.0, 1.0);
}
