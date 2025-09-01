#include "image.h"

void Image::deleter(unsigned char *ptr) { 
	free(ptr); 
}

Image::Image() : 
	m_width(0),
	m_height(0),
	m_channels(0),
	m_bitmap(nullptr, deleter)
{};
Image::Image(const int &_width, const int &_height, const int &_channels) :
	m_width(_width),
	m_height(_height),     
	m_channels(_channels),
	m_bitmap(
		(unsigned char*)calloc(_height*_width*_channels, sizeof(unsigned char)),
		deleter
	)
{
	for (int i = 0; i < m_width*m_height*m_channels; i += m_channels)
		m_bitmap[i + 3] = 255;
}
Image::Image(const int &_width, const int &_height, const int &_channels, const Color &c) :
	m_width(_width),
	m_height(_height),
	m_channels(_channels),
	m_bitmap(
		(unsigned char*)malloc(_height*_width*_channels * sizeof(unsigned char)),
		deleter
	)
{
	for (int i = 0; i < m_height*m_width*m_channels; i += m_channels) {
		m_bitmap[i + 0] = *c.r;
		m_bitmap[i + 1] = *c.g;
		m_bitmap[i + 2] = *c.b;
		m_bitmap[i + 3] = 255;
	}
}
Image::Image(const Image& other) :
	m_width(other.m_width),
	m_height(other.m_height),     
	m_channels(other.m_channels),
	m_bitmap(nullptr, deleter)
{
	if (!other.empty()) {
		m_bitmap.reset(
			(unsigned char*)malloc(m_height*m_width*m_channels * sizeof(unsigned char)),
			deleter
		);
		std::memcpy(m_bitmap.get(), other.m_bitmap.get(), m_height*m_width*m_channels * sizeof(unsigned char));
	}
}
Image::Image(Image&& other) :
	m_width(other.m_width),
	m_height(other.m_height),
	m_channels(other.m_channels),
	m_bitmap(other.m_bitmap)
{
	other.m_width = 0;
	other.m_height = 0;
	other.m_channels = 0;
	other.m_bitmap.reset();	
}
Image::~Image() {} // shared ptr will handle destruction

Image& Image::operator=(const Image& other) {
	m_width = other.m_width;
	m_height = other.m_height;
	m_channels = other.m_channels;
	m_bitmap = other.m_bitmap;
	return *this;
}
Image& Image::operator=(Image&& other) {
	m_bitmap.reset();
	m_width = other.m_width;
	m_height = other.m_height;
	m_channels = other.m_channels;
	m_bitmap = other.m_bitmap;

	other.m_bitmap.reset();
	other.m_height = 0;
	other.m_width = 0;
	other.m_channels = 0;

	return *this;
}

Color Image::operator()(int x, int y) {
	int index = (x + y*m_width) * m_channels;
	assert((void("Index out of range"), index < m_height*m_width*m_channels));
	return Color(
		&m_bitmap[index + 0],
		&m_bitmap[index + 1],
		&m_bitmap[index + 2]
	);
}
Color Image::operator()(const Point &p) {
	return (*this)(p.x, p.y);
}

Image Image::operator*(const float &scalar) {
	Image _tmp(*this);
	for (int i = 0; i < m_height*m_width * m_channels; i += m_channels) {
		for (int j = 0; j < 3; j++) {
			float col = static_cast<float>(_tmp.m_bitmap[i+j]);
			col *= scalar;
			_tmp.m_bitmap[i+j] = static_cast<unsigned char>(std::clamp(static_cast<int>(col), 0, 255));
		}
	}
	return _tmp;
}
Image operator*(const float &scalar, const Image& rhs) {
	Image _tmp(rhs);
	for (int i = 0; i < _tmp.m_height*_tmp.m_width*_tmp.m_channels; i++) {
		float col = static_cast<float>(_tmp.m_bitmap[i]);
		col *= scalar;
		_tmp.m_bitmap[i] = static_cast<unsigned char>(std::clamp(static_cast<int>(col), 0, 255));
	}
	return _tmp;
}
Image Image::operator+(const Image& rhs) {
	assert((
		void("Images of different sizes"), 
		m_height*m_width*m_channels == rhs.m_height*rhs.m_width*rhs.m_channels
	));
	Image _tmp(m_width, m_height, m_channels);
	for (int i = 0; i < m_height*m_width*m_channels; i++) {
		_tmp.m_bitmap[i] = static_cast<unsigned char>(std::clamp((int)m_bitmap[i] + (int)rhs.m_bitmap[i], 0, 255));
	}
	return _tmp;
}

bool Image::empty() const noexcept { return (m_bitmap.get() == nullptr); }
int Image::width() const noexcept { return m_width; }
int Image::height() const noexcept { return m_height; }
int Image::channels() const noexcept { return m_channels; }

void load(Image &image, const std::string &path) {
	if (stbi_info(path.c_str(), &image.m_width, &image.m_height, &image.m_channels)) {
		image.m_bitmap.reset(
			stbi_load(path.c_str(), &image.m_width, &image.m_height, &image.m_channels, 4), 
			Image::deleter
		);
		/*if (image.m_bitmap.get() == nullptr)
			throw "Failed to load image";*/
		image.m_channels = 4;
	}
	else {
		throw "Unsupported file";
	}
}
void write(const Image &image, const std::string& path){
	if (!stbi_write_png(path.c_str(), image.m_width, image.m_height, image.m_channels, image.m_bitmap.get(), 4))
		throw "Failed to write image to disk";
}
void clear(Image &image) {
	image.m_bitmap.reset();
}

unsigned char* Image::get() const{
	return m_bitmap.get();
}