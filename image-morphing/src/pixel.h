#pragma once
#include "imgui/imgui.h"
#include <opencv2/opencv.hpp>
#include <fstream>

class Pixel {
public:
	int x;
	int y;
	Pixel() : x(0), y(0) {};
	Pixel(const int &_x, const int &_y) : x(_x), y(_y) {};
	Pixel(const ImVec2 &p) : x(p.x), y(p.y) {};
	Pixel(const cv::Point2d &p) : x(p.x), y(p.y) {};

	friend std::ostream& operator<<(std::ostream& os, const Pixel &pixel) {
		os << pixel.x << " " << pixel.y;
		return os;
	}

	friend std::istream& operator>>(std::istream& is, Pixel& pixel) {
		is >> pixel.x >> pixel.y;
		return is;
	}

	Pixel& operator+=(const Pixel& other) {
		x += other.x;
		y += other.y;
	}
	Pixel& operator+=(const int& scalar) {
		x += scalar;
		y += scalar;
	}
	Pixel& operator-=(const Pixel& other) {
		x -= other.x;
		y -= other.y;
	}
	Pixel& operator-=(const int& scalar) {
		x -= scalar;
		y -= scalar;
	}
	Pixel& operator*=(const Pixel& other) {
		x *= other.x;
		y *= other.y;
	}
	Pixel& operator*=(const double& scalar) {
		x *= scalar;
		y *= scalar;
	}
	Pixel& operator/=(const Pixel& other) {
		x /= other.x;
		y /= other.y;
	}
	Pixel& operator/=(const double& scalar) {
		x /= scalar;
		y /= scalar;
	}

	friend Pixel operator+(const Pixel& l, const Pixel& r) { return Pixel(l.x + r.x, l.y + r.y); }
	friend Pixel operator+(const Pixel& l, const double& scalar) { return Pixel(l.x + scalar, l.y + scalar); }
	friend Pixel operator+(const double& scalar, const Pixel& pixel) { return pixel + scalar; }

	friend Pixel operator-(const Pixel& l, const Pixel& r) { return Pixel(l.x - r.x, l.y - r.y); }
	friend Pixel operator-(const Pixel& l, const double& scalar) { return l + (-scalar); }
	friend Pixel operator-(const double& scalar, const Pixel& pixel) { return pixel - scalar; }

	friend Pixel operator*(const Pixel& l, const Pixel& r) { return Pixel(l.x * r.x, l.y * r.y); }
	friend Pixel operator*(const Pixel& l, const double& scalar) { return Pixel(l.x * scalar, l.y * scalar); }
	friend Pixel operator*(const double& scalar, const Pixel& pixel) { return pixel * scalar; }

	friend Pixel operator/(const Pixel& l, const Pixel& r) { return Pixel(l.x / r.x, l.y / r.y); }
	friend Pixel operator/(const Pixel& l, const double& scalar) { return Pixel(l.x / scalar, l.y / scalar); }
	friend Pixel operator/(const double& scalar, const Pixel& pixel) { return pixel / scalar; }

	operator ImVec2() const{
		return ImVec2(x, y);
	}

	operator cv::Point2d() const{
		return cv::Point2d(x, y);
	}
};