#version 300 es
precision highp float;

layout (location = 0) in vec2 loc_in;

uniform mat4 ortho;
uniform float pointsize;

out vec2 center;
out float thresh;

void main() {
    gl_Position = ortho * vec4(loc_in+0.5, 0.0, 1.0);
    gl_PointSize = pointsize;
    center = loc_in+0.5;
    thresh = float(gl_PointSize*gl_PointSize)/4.0;
}