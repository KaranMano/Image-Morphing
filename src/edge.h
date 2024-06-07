#pragma once
#include "imgui/imgui.h"
#include <opencv2/opencv.hpp>
#include <fstream>

#include "pixel.h"

class Edge {
public:
	Pixel head, tail;
	bool drawing = true;

	Edge() {};
	Edge(Pixel t) : tail(t) {};
	Edge(Pixel t, Pixel h, bool d) : tail(t), head(h), drawing(d) {};

	friend std::ostream& operator<<(std::ostream& os, const Edge &edge) {
		os << edge.tail << " "
			<< edge.head << " "
			<< edge.drawing;
		return os;
	}

	friend std::istream& operator>>(std::istream& is, Edge& edge) {
		is >> edge.tail
			>> edge.head
			>> edge.drawing;
		return is;
	}

	void setHead(Pixel t) {
		head = t;
		drawing = false;
	}
};