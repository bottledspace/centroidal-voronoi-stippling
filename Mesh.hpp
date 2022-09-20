#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

template <typename T>
struct VertexDescriptor {};

template <typename Vertex>
struct IndexedVertexBuffer {
    int count;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    IndexedVertexBuffer()
        : count(0), vao(0), vbo(0), ibo(0)
        {}

    void compile(const std::vector<Vertex> &verts,
                 const std::vector<unsigned> &indices) {
        if (!vao) {
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ibo);
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            VertexDescriptor<Vertex>::describe();
            
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            verts.size()*sizeof(Vertex),
            verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size()*sizeof(unsigned),
            indices.data(), GL_STATIC_DRAW);
        count = indices.size();
    }

    void draw(int primtype = GL_TRIANGLES) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(primtype, count, GL_UNSIGNED_INT, (void *)0);
    }
    void draw(int ntimes, int primtype) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElementsInstanced(primtype, count, GL_UNSIGNED_INT, (void *)0, ntimes);
    }
};

template <typename Vertex>
struct VertexBuffer {
    int count;
    GLuint vao;
    GLuint vbo;

    VertexBuffer()
        : count(0), vao(0), vbo(0)
        {}

    void compile(const std::vector<Vertex> &verts) {
        if (!vao) {
            glGenBuffers(1, &vbo);
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            VertexDescriptor<Vertex>::describe();
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            verts.size()*sizeof(Vertex),
            verts.data(), GL_STATIC_DRAW);
        count = verts.size();
    }

    void draw(int primtype = GL_TRIANGLES) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(primtype, 0, count);
    }
    void draw(int ntimes, int primtype) const {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArraysInstanced(primtype, 0, count, ntimes);
    }
};

template <typename Vertex>
class IndexedMesh {
public:
    const Vertex &vert(unsigned i) const {
        return m_verts.at(i);
    }

    unsigned add_vert(Vertex &&v) {
        m_verts.push_back(std::move(v));
        return m_verts.size()-1;
    }
    unsigned add_face(unsigned a, unsigned b, unsigned c) {
        m_indices.insert(m_indices.end(), {a,b,c});
        return m_indices.size()-3;
    }
    
    IndexedVertexBuffer<Vertex> &buffer() {
        m_vb.compile(m_verts, m_indices);
        return m_vb;
    }
private:
    std::vector<Vertex> m_verts;
    std::vector<unsigned> m_indices;
    IndexedVertexBuffer<Vertex> m_vb;
};

template <>
struct VertexDescriptor<glm::vec3> {
    static void describe() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(glm::vec3), (void *)0);
    }
};
template <>
struct VertexDescriptor<glm::vec2> {
    static void describe() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            sizeof(glm::vec2), (void *)0);
    }
};