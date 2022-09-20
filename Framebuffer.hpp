#pragma once

#include "Texture.hpp"
#include <map>
#include <GL/glew.h>

class Framebuffer {
public:
    Framebuffer()
        : m_fbo(0), m_hasdepth(false) {}

    bool complete() {
        if (!m_fbo)
		    glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        m_hasdepth = false;
        for (const auto &channel : m_channels) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, channel.first,
                GL_TEXTURE_2D, channel.second.texid(), 0);
            if (channel.first == GL_DEPTH_ATTACHMENT)
                m_hasdepth = true;
        }
        const GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7,
            GL_COLOR_ATTACHMENT8,
            GL_COLOR_ATTACHMENT9,
            GL_COLOR_ATTACHMENT10,
            GL_COLOR_ATTACHMENT11,
            GL_COLOR_ATTACHMENT12,
            GL_COLOR_ATTACHMENT13,
            GL_COLOR_ATTACHMENT14,
            GL_COLOR_ATTACHMENT15,
        };
        glDrawBuffers(numcolors(), buffers);
        return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }

    void bind(int x, int y, int width, int height) const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(x, y, width, height);
    }
    void bind() const {
        const auto &first = m_channels.at(GL_COLOR_ATTACHMENT0);
        bind(0, 0, first.width(), first.height());
    }


	Texture &channel(GLenum channel) {
        return m_channels[channel];
    }
private:
    int numcolors() const {
        return m_hasdepth? (m_channels.size()-1):m_channels.size();
    }

    GLuint m_fbo;
    bool m_hasdepth;
    std::map<GLenum, Texture> m_channels;
};