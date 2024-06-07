#pragma once
#include <fstream>

#include "point.h"

class Edge {
public:
	Point head, tail;
	bool drawing = true;

	Edge() {};
	Edge(Point t) : tail(t) {};
	Edge(Point t, Point h, bool d) : tail(t), head(h), drawing(d) {};

	friend std::ostream& operator<<(std::ostream& os, const Edge &edge);
	friend std::istream& operator>>(std::istream& is, Edge& edge);

	void setHead(Point h);
};