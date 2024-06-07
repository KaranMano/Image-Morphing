#include "edge.h"

std::ostream& operator<<(std::ostream& os, const Edge &edge) {
	os << edge.tail << " "
		<< edge.head << " "
		<< edge.drawing;
	return os;
}
std::istream& operator>>(std::istream& is, Edge& edge) {
	is >> edge.tail
		>> edge.head
		>> edge.drawing;
	return is;
}

void Edge::setHead(Point h) {
	head = h;
	drawing = false;
}
