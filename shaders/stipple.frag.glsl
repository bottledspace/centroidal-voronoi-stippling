#version 300 es
precision highp float;

layout (location = 0) out vec4 color;

in vec2 center;
in float thresh;

void main() {
    vec2 disp = center - gl_FragCoord.xy;
    if (dot(disp,disp) > thresh)
        discard;
    color = vec4(0.0,0.0,0.0,1.0);
}