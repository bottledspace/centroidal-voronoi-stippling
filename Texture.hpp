#pragma once

#include <GL/glew.h>
#include <vector>
#include <map>
#include <glm/gtc/type_ptr.hpp>

class Texture {
public:
	Texture()
	: m_texid(0)
		{}
	
    void create(int width, int height, GLenum intformat = GL_RGBA32F, GLenum format = GL_RGBA) {
		if (m_texid)
			return; // Already created!
		m_width = width;
		m_height = height;

		glGenTextures(1, &m_texid);
		glBindTexture(GL_TEXTURE_2D, m_texid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, intformat, m_width, m_height, 0,
			format,
			GL_UNSIGNED_BYTE, NULL);
    }
    void bind(GLenum slot = GL_TEXTURE0) const {
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, m_texid);
    }

	void subimage(glm::ivec2 pos, glm::ivec2 size, GLenum format, GLenum type, const void *pixels) {
		glBindTexture(GL_TEXTURE_2D, m_texid);
		glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y,
			size.x, size.y, format, type, pixels);
	}

    GLuint texid() const
        { return m_texid; }
	bool valid() const
		{ return m_texid != 0; }
	GLsizei width() const
		{ return m_width; }
	GLsizei height() const
		{ return m_height; }
private:
    GLuint m_texid;
    GLsizei m_width, m_height;
};