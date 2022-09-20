#pragma once

#include "Framebuffer.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


struct Voronoi {
    void create(int width, int height);
    void compute(const std::vector<glm::vec2> &points);

    unsigned operator ()(size_t x, size_t y) const
        { return pixels[width*y+x]; }

private:
    int width, height;
    Shader shader;
    IndexedVertexBuffer<glm::vec3> cone;
    Framebuffer framebuf;
    Texture pointbuf;
    std::vector<unsigned> pixels;
};

void Voronoi::create(int width, int height) {
    this->width = width;
    this->height = height;

    // Create point buffer.
    pointbuf.create(512, 512, GL_RG32F, GL_RG);

    // Create cone mesh.
    const int N = 64;
    IndexedMesh<glm::vec3> mesh;
    int first = mesh.add_vert(glm::vec3(0.0f,0.0f,0.0f));
    int prev = mesh.add_vert(glm::vec3(1024.0f,0.0f,-1024.0f));
    for (int k = 1; k <= N; k++) {
        float theta = k * 2*M_PI/float(N);
        int next = mesh.add_vert(glm::vec3(1024.0f*cos(theta), 1024.0f*sin(theta), -1024.0f));
        mesh.add_face(prev, first, next);
        prev = next;
    }
    cone = mesh.buffer();

    // Compile shader for drawing cones.
    shader.create("../shaders/voronoi");

    // Create framebuffer (with depth buffer).
    framebuf.channel(GL_COLOR_ATTACHMENT0)
        .create(width,height,GL_R32UI,GL_RED_INTEGER);
    framebuf.channel(GL_DEPTH_ATTACHMENT)
        .create(width,height,GL_DEPTH_COMPONENT32F,GL_DEPTH_COMPONENT);
    if (!framebuf.complete())
        std::cerr << "Warn: Failed to create framebuffer." << std::endl;

    // Create pixel buffer for reads.
    pixels.resize(width*height);
}

void Voronoi::compute(const std::vector<glm::vec2> &points) {
    assert(points.size() <= pointbuf.width()*pointbuf.height());

    // Send the current positions as a texture.
    unsigned rows = points.size()/pointbuf.width();
    pointbuf.subimage({0,0}, {pointbuf.width(), rows},
        GL_RG, GL_FLOAT, points.data());
    glFinish();

    // Draw the points as right cones using the depthbuffer to sort.
    framebuf.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    shader.use();
    shader.uniform("ortho", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1024.0f));
    shader.uniform("pointbuf", pointbuf);
    cone.draw(points.size(), GL_TRIANGLES);

    // Read back diagram from framebuffer.
    glFinish();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, pixels.data());
}
