#pragma once

#include "config.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>
#include <fstream>
#include <cmath>
#include <future>
#include <chrono>

#include "core/image.h"
#include "core/point.h"
#include "core/edge.h"

void clampToImage(Point &p, const Image &image);
inline Point generateIntermediatePoint(const Point &source, const Point &dest, const float &step);

void loadImageWindow(
	Image &image,
	Texture &texture,
	std::vector<Edge> &edges,
	std::string &buffer,
	const std::string &title
);
void displayImage(
	const Image &image,
	const Texture &texture,
	std::vector<Edge> &edges, 
	const std::string title, 
	bool addHandle, 
	bool eraseHandle
);
Image morphImage(
	const Image &sourceImage,
	const Image &targetImage, 
	const std::vector<Edge> &sourceEdges,
	const std::vector<Edge> &targetEdges,
	const float step
);
Image generateMorph(
	const Image &sourceImage, 
	const Image &targetImage, 
	const std::vector<Edge> &sourceEdges,
	const std::vector<Edge> &targetEdges,
	const int step, 
	const int steps
);
void loadEdges(std::vector<Edge> &edges, const std::string &path);
void saveEdges(std::vector<Edge> &edges, const std::string &path);
// void generateFeatures(Image &image, int index);
