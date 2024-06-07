#include "point.h"

std::ostream& operator<<(std::ostream& os, const Point &pixel) {
	os << pixel.x << " " << pixel.y;
	return os;
}
std::istream& operator>>(std::istream& is, Point& pixel) {
	is >> pixel.x >> pixel.y;
	return is;
}

bool Point::operator==(const Point& r) const {
	return x == r.x && y == r.y;
}
bool Point::operator!=(const Point& r) const {
	return !(*this == r);
}
Point& Point::operator+=(const Point& r) {
	x += r.x;
	y += r.y;
	return *this;
}
Point& Point::operator+=(const float& scalar) {
	x += scalar;
	y += scalar;
	return *this;
}

Point& Point::operator-=(const Point& r) {
	x += -r.x;
	y += -r.y;
	return *this;
}
Point& Point::operator-=(const float& scalar) {
	x += -scalar;
	y += -scalar;
	return *this;
}

Point& Point::operator*=(const Point& r) {
	x *= r.x;
	y *= r.y;
	return *this;
}
Point& Point::operator*=(const float& scalar) {
	x *= scalar;
	y *= scalar;
	return *this;
}

Point& Point::operator/=(const Point& r) {
	x *= 1 / r.x;
	y *= 1 / r.y;
	return *this;
}
Point& Point::operator/=(const float& scalar) {
	x *= 1 / scalar;
	y *= 1 / scalar;
	return *this;
}

Point Point::operator+(const Point& r) const {
	Point tmp = *this;
	return (tmp += r);
}
Point Point::operator+(const float& scalar) const {
	Point tmp = *this;
	return (tmp += scalar);
}
Point operator+(const float& scalar, const Point& pixel) { return pixel + scalar; }

Point Point::operator-() const { return { -x, -y }; }
Point Point::operator-(const Point& r) const {
	Point tmp = *this;
	return (tmp -= r);
}
Point Point::operator-(const float& scalar) const {
	Point tmp = *this;
	return (tmp -= scalar);
}
Point operator-(const float& scalar, const Point& pixel) { return pixel - scalar; }

Point Point::operator*(const Point& r) const {
	Point tmp = *this;
	return (tmp *= r);
}
Point Point::operator*(const float& scalar) const {
	Point tmp = *this;
	return (tmp *= scalar);
}
Point operator*(const float& scalar, const Point& pixel) { return pixel * scalar; }

Point Point::operator/(const Point& r) const {
	Point tmp = *this;
	return (tmp /= r);
}
Point Point::operator/(const float& scalar) const {
	Point tmp = *this;
	return (tmp /= scalar);
}
Point operator/(const float& scalar, const Point& pixel) { return pixel / scalar; }

Point::operator ImVec2() const { return { static_cast<float>(x), static_cast<float>(y) }; }

float Point::norm() const { return std::sqrt(x * x + y * y); }
float Point::dot(const Point &r) const { return x * r.x + y * r.y; }
Point Point::perp() const { return { y, -x }; }
Point Point::cast() const { return { std::round(x), std::round(y) }; }
