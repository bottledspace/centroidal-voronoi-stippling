#version 300 es
precision highp float;

layout (location = 0) out uint color;

flat in uint pointid;

void main() {
    color = pointid;
}
