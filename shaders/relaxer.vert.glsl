#version 300 es
precision highp float;

layout (location = 0) in vec2 loc_in;

uniform sampler2D density;
uniform highp usampler2D voronoi;

out vec4 color;

void main() {
    /*ivec2 inloc = ivec2(gl_InstanceID%1024, gl_InstanceID/1024);
    uint id = texelFetch(voronoi, inloc, 0).r;
    float p = texelFetch(density, inloc, 0).r;

    gl_Position = vec4(id%512u, id/512u,0.0,1.0);
    color = vec4(p*vec2(inloc.xy),p,1.0);*/

    gl_Position = vec4(loc_in, 0.0, 1.0);
    color = vec4(1.0,1.0,1.0,1.0);
}