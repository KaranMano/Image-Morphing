#pragma once
#include "../imgui/imgui.h"
#include <fstream>
#include <cmath>

class Point {
public:
	float x;
	float y;
	Point() : x(0), y(0) {};
	Point(const float &_x, const float &_y) : x(_x), y(_y) {};
	Point(const ImVec2 &p) : x(p.x), y(p.y) {};

	friend std::ostream& operator<<(std::ostream& os, const Point &pixel);
	friend std::istream& operator>>(std::istream& is, Point& pixel);

	bool operator==(const Point& r) const;
	bool operator!=(const Point& r) const;

	Point& operator+=(const Point& r);
	Point& operator+=(const float& scalar);

	Point& operator-=(const Point& r);
	Point& operator-=(const float& scalar);

	Point& operator*=(const Point& r);
	Point& operator*=(const float& scalar);

	Point& operator/=(const Point& r);
	Point& operator/=(const float& scalar);

	Point operator+(const Point& r) const;
	Point operator+(const float& scalar) const;
	friend Point operator+(const float& scalar, const Point& pixel);

	Point operator-() const;
	Point operator-(const Point& r) const;
	Point operator-(const float& scalar) const;
	friend Point operator-(const float& scalar, const Point& pixel);

	Point operator*(const Point& r) const;
	Point operator*(const float& scalar) const;
	friend Point operator*(const float& scalar, const Point& pixel);

	Point operator/(const Point& r) const;
	Point operator/(const float& scalar) const;
	friend Point operator/(const float& scalar, const Point& pixel);

	operator ImVec2() const;

	float norm() const;
	float dot(const Point &r) const;
	Point perp() const;
	Point cast() const;
};