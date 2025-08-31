#pragma once

#include <stb_image.h>
#include <stb_image_write.h>

#include <vector>
#include <cassert>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <string>
#include <cstring>
#include <iostream>
#include <memory>
#include <functional>

#include "edge.h"
#include "color.h"
#include "point.h"
#include "texture.h"

class Image {
private:
	static void deleter(unsigned char *ptr);
	std::shared_ptr<unsigned char[]> m_bitmap;
	int m_width, m_height, m_channels;

public:
	Image();
	Image(const int &_width, const int &_height, const int &_channels);
	Image(const int &_width, const int &_height, const int &_channels, const Color &c);
	Image(const Image& other);
	Image(Image&& other);
	~Image();

	Image& operator=(const Image& other);
	Image& operator=(Image&& other);

	Color operator()(int x, int y);
	Color operator()(const Point &p);

	Image operator*(const float &scalar);
	friend Image operator*(const float &scalar, const Image& rhs);
	Image operator+(const Image& rhs);

	bool empty() const noexcept;
	int width() const noexcept;
	int height() const noexcept;
	int channels() const noexcept;
	
	friend void clear(Image &image);
	friend void load(Image &image, const std::string &path);
	friend void write(const Image &image, const std::string& path);
	friend void load(Texture &texture, const Image& image);
};
