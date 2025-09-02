#include "texture.h"
#include "image.h"
#include <GL/gl.h>

void Texture::deleter(GLuint *ptr) {
	if (ptr) glDeleteTextures(1, ptr); 
}

Texture::Texture() : m_texture(nullptr, deleter) {}
Texture::Texture(const Texture &other) { m_texture = other.m_texture; }
Texture::Texture(Texture&& other) {
	m_texture = other.m_texture;
	other.m_texture.reset();
}
Texture::~Texture() {}

Texture& Texture::operator=(const Texture& other) {
	m_texture = other.m_texture;
	return *this;
}
Texture& Texture::operator=(Texture&& other) {
	m_texture = other.m_texture;
	other.m_texture.reset();
	return *this;
}

void clear(Texture &texture) {
	texture.m_texture.reset();
}
void load(Texture &texture, const Image& image) {
	texture.m_texture.reset(new GLuint(0), Texture::deleter);
	glGenTextures(1, texture.m_texture.get());
	glBindTexture(GL_TEXTURE_2D, *texture.m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.m_width, image.m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.m_bitmap.get());
	glBindTexture(GL_TEXTURE_2D, 0);
}
bool Texture::empty() const { return m_texture.get() == nullptr; }
GLuint Texture::get() const { return (m_texture.get()) ? *m_texture : 0; }
void Texture::set(GLuint texID) { 
	m_texture.reset(new GLuint(texID), Texture::deleter);
}
