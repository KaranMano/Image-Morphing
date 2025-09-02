#include "utils.h"

#include <iostream>

bool errorLastFrame = false;

void clampToImage(Point &p, const Image &image) {
	p.x = std::clamp((int)p.x, 0, image.width() - 1);
	p.y = std::clamp((int)p.y, 0, image.height() - 1);
}

inline Point generateIntermediatePoint(const Point &source, const Point &dest, const float &step) {
	return (dest - source) * step + source;
}

void loadImageWindow(
	Image &image,
	Texture &texture,
	std::vector<Edge> &edges,
	std::string &buffer,
	const std::string &title
) {
	ImGui::Begin(title.c_str());
	ImGui::InputText(("input##" + title).c_str(), &buffer);

	if (ImGui::Button(("submit##" + title).c_str())) {
		try {
			errorLastFrame = load(image, buffer) < 0;
			load(texture, image);
		}
		catch (std::string errorMessage) {
			std::cout << errorMessage << std::endl;
		}
	}
	if (ImGui::Button(("clear##" + title).c_str()))
		edges.clear();
	ImGui::End();
}

Image morphImage(
	const Image &sourceImage,
	const Image &targetImage, 
	const std::vector<Edge> &sourceEdges,
	const std::vector<Edge> &targetEdges,
	const float step
) {
	Image dst(sourceImage.height(), sourceImage.width(), sourceImage.channels());
	float a = 10, b = 1.5, p = 0.2;
	//! gpu optimisation maybe
	for (int col = 0; col < targetImage.width(); col++) {
		for (int row = 0; row < targetImage.height(); row++) {
			Point destX(col, row);
			Point dispSum(0, 0);
			float weightSum = 0;

			for (int i = 0; i < targetEdges.size(); i++) {
				Point destP = targetEdges[i].head;
				Point destQ = targetEdges[i].tail;
				Point sourceP = sourceEdges[i].head;
				Point sourceQ = sourceEdges[i].tail;

				destP = generateIntermediatePoint(sourceP, destP, step);
				destQ = generateIntermediatePoint(sourceQ, destQ, step);

				float u = (destX - destP).dot(destQ - destP) / ((destQ - destP).norm() * (destQ - destP).norm());
				float v = (destX - destP).dot((destQ - destP).perp()) / (destQ - destP).norm();

				Point sourceX = sourceP + u * (sourceQ - sourceP) + (v * (sourceQ - sourceP).perp()) / (sourceQ - sourceP).norm();
				Point disp = sourceX - destX;

				float dist;
				if (u >= 1)
					dist = (destX - destQ).norm();
				else if (u <= 0)
					dist = (destX - destP).norm();
				else
					dist = std::abs(v);

				float weight = std::pow(std::pow((destQ - destP).norm(), p) / (a + dist), b);

				weightSum += weight;
				dispSum += disp * weight;
			}
			Point finalX = destX + dispSum / weightSum;
			clampToImage(finalX, sourceImage);
			dst(destX) = const_cast<Image&>(sourceImage)(static_cast<int>(std::trunc(finalX.x)), static_cast<int>(std::trunc(finalX.y)));
		}
	}
	return dst;
}

Image generateMorph(
	const Image &sourceImage, 
	const Image &targetImage, 
	const std::vector<Edge> &sourceEdges,
	const std::vector<Edge> &targetEdges,
	const int step, 
	const int steps
) {
	Image finalMorph;
	if (step == steps)
		finalMorph = targetImage;
	else if (step == 0)
		finalMorph = sourceImage;
	else {
		float alpha = (float)step / steps;
		Image sourceMorph = morphImage(targetImage, sourceImage, targetEdges, sourceEdges, 1 - alpha);
		Image destMorph = morphImage(sourceImage, targetImage, sourceEdges, targetEdges, alpha);
		finalMorph = (alpha)* sourceMorph + (1 - alpha)* destMorph;
	}
	errorLastFrame = write(finalMorph, "morph" + std::to_string(step) + ".png") < 0;
	return finalMorph;
}

void addHandles(
	const Image &image, 
	std::vector<Edge> &edges,
	Point &topLeft, 
	Point &bottomRight, 
	float &scaleFactor
) {
	ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, IM_COL32(0, 255, 0, 255), 0, 0, 5);
	if (ImGui::IsItemClicked()) {
		Point mousePos = ImGui::GetMousePos();
		Point clickPos = (mousePos - topLeft) / scaleFactor;
		if (edges.size() > 0 && edges.back().drawing)
			edges.back().setHead(clickPos);
		else {
			if (edges.size() > 0 && edges.back().drawing)
				edges.pop_back();
			edges.emplace_back(clickPos);
		}
	}
}

void eraseHandles(
	const Image &image,
	std::vector<Edge> &edges, 
	Point &topLeft, 
	Point  &bottomRight, 
	float scaleFactor
) {
	ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, IM_COL32(255, 0, 0, 255), 0, 0, 5);
	for (auto it = edges.begin(); it != edges.end();) {
		Edge &edge = *it;
		Point tail = edge.tail * scaleFactor + topLeft;
		Point head = edge.head * scaleFactor + topLeft;

		int height = 10, width = 5;
		Point line = (head - tail) / (head - tail).norm();
		Point perpendicular = line.perp();

		if (ImGui::IsMouseClicked(0)) {
			Point mousePos = ImGui::GetMousePos();
			if (std::abs((mousePos - tail).dot(line)) < (head - tail).norm()
				&& std::abs((mousePos - tail).dot(perpendicular)) < width)
			{
				it = edges.erase(it);
			}
			else { it++; }
		}
		else { it++; }
	}
}

void drawEdges(
	const Image &image, 
	std::vector<Edge> &edges, 
	Point &topLeft, 
	float scaleFactor
) {
	for (auto &edge : edges) {
		Point tail = edge.tail * scaleFactor + topLeft;
		Point head = (edge.drawing) ? Point(ImGui::GetMousePos()) : edge.head * scaleFactor + topLeft;

		int height = 10, width = 5;
		Point line = (head - tail) / (head - tail).norm();
		Point perpendicular = line.perp();

		ImGui::GetWindowDrawList()->AddTriangleFilled(	// arrowhead
			head + line,
			head - line * height + perpendicular * width,
			head - line * height - perpendicular * width,
			IM_COL32(255, 0, 0, 255));

		ImGui::GetWindowDrawList()->AddLine(tail, head - line * 10, IM_COL32(255, 0, 0, 255), 2);
	}
}

void displayImage(
	const Image &image,
	const Texture &texture,
	std::vector<Edge> &edges, 
	const std::string title, 
	bool addHandle, 
	bool eraseHandle
) {

	ImGui::Begin(title.c_str());
	
	if (!texture.empty()) {
		ImVec2 winSize = ImGui::GetWindowSize();	// store in variable to ensure usage of same window size
		float scaleFactor = std::min(winSize.y / image.width(), winSize.x / image.height());
		ImGui::Image(
			reinterpret_cast<void*>(static_cast<intptr_t>(texture.get())),
			ImVec2(image.width() * scaleFactor, image.height() * scaleFactor)
		);

		Point topLeft = ImGui::GetItemRectMin(), bottomRight = ImGui::GetItemRectMax();

		if (addHandle) addHandles(image, edges, topLeft, bottomRight, scaleFactor);
		drawEdges(image, edges, topLeft, scaleFactor);
		if (eraseHandle) eraseHandles(image, edges, topLeft, bottomRight, scaleFactor);
	}
	ImGui::End();
}


void loadEdges(std::vector<Edge> &edges, const std::string &path) {
	edges.clear();
	std::ifstream edgeCache(path);

	if (edgeCache.good()) {
		while (!edgeCache.eof()) {
			edges.emplace_back();
			edgeCache >> edges.back();
		}
	}
}
void saveEdges(std::vector<Edge> &edges, const std::string &path) {
	std::ofstream edgeCache(path);

	for (const auto &edge : edges)
		edgeCache << " " << edge;
}