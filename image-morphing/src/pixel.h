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

	bool operator==(const Pixel& r) const{
		return x == r.x && y == r.y;
	}
	bool operator!=(const Pixel& r) const{
		return !(*this == r);
	}
	Pixel& operator+=(const Pixel& r) {
		x += r.x;
		y += r.y;
		return *this;
	}
	Pixel& operator+=(const int& scalar) {
		x += scalar;
		y += scalar;
		return *this;
	}
	Pixel& operator-=(const Pixel& r) {
		x += -r.x;
		y += -r.y;
		return *this;
	}
	Pixel& operator-=(const int& scalar) {
		x += -scalar;
		y += -scalar;
		return *this;
	}
	Pixel& operator*=(const Pixel& r) {
		x *= r.x;
		y *= r.y;
		return *this;
	}
	Pixel& operator*=(const double& scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}
	Pixel& operator/=(const Pixel& r) {
		x *= 1 / r.x;
		y *= 1 / r.y;
		return *this;
	}
	Pixel& operator/=(const double& scalar) {
		x *= 1 / scalar;
		y *= 1 / scalar;
		return *this;
	}

	Pixel operator+(const Pixel& r) const {
		Pixel tmp = *this;
		return (tmp += r);
	}
	Pixel operator+(const double& scalar) const {
		Pixel tmp = *this;
		return (tmp += scalar);
	}
	friend Pixel operator+(const double& scalar, const Pixel& pixel) { return pixel + scalar; }

	Pixel operator-() const {
		return Pixel(-x, -y);
	}
	Pixel operator-(const Pixel& r) const {
		Pixel tmp = *this;
		return (tmp -= r);
	}
	Pixel operator-(const double& scalar) const {
		Pixel tmp = *this;
		return (tmp -= scalar);
	}
	friend Pixel operator-(const double& scalar, const Pixel& pixel) { return pixel - scalar; }

	Pixel operator*(const Pixel& r) const {
		Pixel tmp = *this;
		return (tmp *= r);
	}
	Pixel operator*(const double& scalar) const {
		Pixel tmp = *this;
		return (tmp *= scalar);
	}
	friend Pixel operator*(const double& scalar, const Pixel& pixel) { return pixel * scalar; }

	Pixel operator/(const Pixel& r) const{
		Pixel tmp = *this;
		return (tmp /= r);
	}
	Pixel operator/(const double& scalar) const{
		Pixel tmp = *this;
		return (tmp /= scalar);
	}
	friend Pixel operator/(const double& scalar, const Pixel& pixel) { return pixel / scalar; }

	operator ImVec2() const {
		return ImVec2(x, y);
	}

	operator cv::Point2d() const {
		return cv::Point2d(x, y);
	}
};