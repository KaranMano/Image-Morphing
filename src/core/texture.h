#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>

class Image;

class Texture {
private:
	std::shared_ptr<GLuint> m_texture;
	static void deleter(GLuint *ptr);

public:
	Texture();
	Texture(const Texture &other);
	Texture(Texture&& other);
	~Texture();

	Texture& operator=(const Texture& other);
	Texture& operator=(Texture&& other);
	bool empty() const;
	GLuint get() const;
	friend void clear(Texture &texture);
	friend void load(Texture &texture, const Image& image);
};