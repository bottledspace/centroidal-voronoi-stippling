#version 300 es
precision highp float;

layout (location = 0) out vec4 color_out;

in vec4 color;

void main() {
    color_out = color;
}