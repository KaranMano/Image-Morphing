#pragma once

class Color {
public:
	bool referenced;
	unsigned char *r, *g, *b;

	Color() : r(nullptr), g(nullptr), b(nullptr), referenced(false) {}
	Color(unsigned char _r, unsigned char _g, unsigned char _b)
		: referenced(false)
	{
		r = new unsigned char;
		g = new unsigned char;
		b = new unsigned char;
		*r = _r;
		*g = _g;
		*b = _b;
	}
	Color(unsigned char* _r, unsigned char* _g, unsigned char* _b)
		: r(_r), g(_g), b(_b), referenced(true)
	{}
	Color(Color &&other) : referenced(true) {
		r = other.r;
		g = other.g;
		b = other.b;
	}
	~Color() {
		if (!referenced) {
			delete r;
			delete g;
			delete b;
		}
	}

	Color& operator=(const Color &other) {
		*r = *(other.r);
		*g = *(other.g);
		*b = *(other.b);
		return *this;
	}
};