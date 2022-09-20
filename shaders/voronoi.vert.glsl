#version 300 es
precision highp float;

layout (location = 0) in vec3 loc_in;

uniform mat4 ortho;
uniform sampler2D pointbuf;

flat out uint pointid;

void main() {
    pointid = uint(gl_InstanceID);
    vec2 ofs = texelFetch(pointbuf, ivec2(gl_InstanceID%512, gl_InstanceID/512), 0).xy;
    gl_Position = ortho * vec4(loc_in.xy + ofs + 0.5, loc_in.z, 1.0);
}
