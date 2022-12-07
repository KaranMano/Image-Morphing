#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_stdlib.h"

#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>
#include <fstream>
#include <cmath>

class Edge {
public:
	ImVec2 head;
	ImVec2 tail;
	bool drawing;

	Edge() {};
	Edge(ImVec2 t) : tail(t), drawing(true) {};
	Edge(ImVec2 t, ImVec2 h, bool d) : tail(t), head(h), drawing(d) {};

	void setHead(ImVec2 t) {
		head = t;
		drawing = false;
	}
};

bool editHandle = false, reloadMorphed = false;
std::vector<Edge> sourceEdges;
std::vector<Edge> targetEdges;

cv::Point2d perp(const cv::Point2d &v) {
	cv::Point3d planePerp(0, 0, 1);
	cv::Point3d p = cv::Point3d(v.x, v.y, 0).cross(planePerp);
	return cv::Point2d(p.x, p.y);
}

cv::Point2d convToPoint2d(const ImVec2 &p) {
	return cv::Point2d(p.x, p.y);
}

ImVec2 convToImVec2(const cv::Point2d &p) {
	return ImVec2(p.x, p.y);
}

void writeEdges(std::string sourcePath, std::string targetPath) {
	std::ofstream sourceLog("./source.edges");
	std::ofstream targetLog("./target.edges");

	for (const auto &edge : sourceEdges) {
		sourceLog << edge.tail.x << " "
			<< edge.tail.y << " "
			<< edge.head.x << " "
			<< edge.head.y << " "
			<< edge.drawing << std::endl;
	}

	for (const auto &edge : targetEdges) {
		targetLog << edge.tail.x << " "
			<< edge.tail.y << " "
			<< edge.head.x << " "
			<< edge.head.y << " "
			<< edge.drawing << std::endl;
	}
}

void readEdges() {
	targetEdges.clear();
	sourceEdges.clear();
	std::ifstream sourceLog("./source.edges");
	std::ifstream targetLog("./target.edges");

	if (sourceLog.good()) {
		while (!sourceLog.eof()) {
			Edge currEdge;
			sourceLog >> currEdge.tail.x
				>> currEdge.tail.y
				>> currEdge.head.x
				>> currEdge.head.y
				>> currEdge.drawing;
			if (!sourceLog.eof())
				sourceEdges.push_back(currEdge);
		}
	}

	if (targetLog.good()) {
		while (!targetLog.eof()) {
			Edge currEdge;
			targetLog >> currEdge.tail.x
				>> currEdge.tail.y
				>> currEdge.head.x
				>> currEdge.head.y
				>> currEdge.drawing;
			if (!targetLog.eof())
				targetEdges.push_back(currEdge);
		}
	}
	std::cout << "edges read\n";
}

void printEdges() {

	std::cout << sourceEdges.size() << "\n";
	for (const auto &edge : sourceEdges) {
		std::cout << edge.tail.x << " "
			<< edge.tail.y << " "
			<< edge.head.x << " "
			<< edge.head.y << " "
			<< edge.drawing << std::endl;
	}

	std::cout << targetEdges.size() << "\n";
	for (const auto &edge : targetEdges) {
		std::cout << edge.tail.x << " "
			<< edge.tail.y << " "
			<< edge.head.x << " "
			<< edge.head.y << " "
			<< edge.drawing << std::endl;
	}
}

void loadImage(cv::Mat &image, GLuint &texture, const std::string &path) {
	image = cv::imread(path, cv::IMREAD_COLOR);
	if (!image.empty()) {
		cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
		cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
	}
}

void loadImageWindow(cv::Mat &image, std::string &buffer, GLuint &texture, const std::string &title) {
	ImGui::Begin(title.c_str());
	ImGui::InputText(("input##" + title).c_str(), &buffer);

	if (ImGui::Button(("submit##" + title).c_str()))
		loadImage(image, texture, buffer);
	if (ImGui::Button(("clear##" + title).c_str()))
		(title == "source path") ? sourceEdges.clear() : targetEdges.clear();
	ImGui::End();
}

void displayImage(cv::Mat image, GLuint texture, std::string title) {
	std::vector<Edge> *edges = nullptr;
	edges = (title == "source image") ? &sourceEdges : &targetEdges;

	ImGui::Begin(title.c_str());
	if (!image.empty()) {
		ImVec2 winSize = ImGui::GetWindowSize();	// store in variable to ensure usage of same window size
		double scaleFactor = std::min(winSize.y / image.rows, winSize.x / image.cols);
		ImGui::Image(
			reinterpret_cast<void*>(static_cast<intptr_t>(texture)),
			ImVec2(image.cols * scaleFactor, image.rows * scaleFactor)
		);
		ImVec2 topLeft = ImGui::GetItemRectMin(), bottomRight = ImGui::GetItemRectMax();

		if (editHandle) {
			ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, IM_COL32(255, 0, 0, 255), 0, 0, 5);
			if (ImGui::IsItemClicked()) {
				ImVec2 mousePos = ImGui::GetMousePos();
				ImVec2 clickPos = ImVec2(
					std::trunc((mousePos.x - topLeft.x) / scaleFactor),
					std::trunc((mousePos.y - topLeft.y) / scaleFactor)
				);
				if (edges->size() > 0 && edges->back().drawing)
					edges->back().setHead(clickPos);
				else
					edges->push_back(Edge(clickPos));
			}
		}

		for (auto &edge : *edges) {
			ImVec2 tail = ImVec2(edge.tail.x * scaleFactor + topLeft.x, edge.tail.y * scaleFactor + topLeft.y);
			ImVec2 head = (edge.drawing) ?
				ImGui::GetMousePos()
				:
				ImVec2(edge.head.x * scaleFactor + topLeft.x, edge.head.y * scaleFactor + topLeft.y);

			int height = 10, width = 5;
			cv::Point2d h = convToPoint2d(head);
			cv::Point2d t = convToPoint2d(tail);
			cv::Point2d line = (t - h) / cv::norm(h - t);
			cv::Point2d linePerp = perp(line);

			ImGui::GetWindowDrawList()->AddTriangleFilled(	// arrowhead
				convToImVec2(h),
				convToImVec2(h + line * height + linePerp * width),
				convToImVec2(h + line * height - linePerp * width),
				IM_COL32(255, 0, 0, 255));

			ImGui::GetWindowDrawList()->AddLine(tail, convToImVec2(h + line * 10), IM_COL32(255, 0, 0, 255), 2);
		}
	}
	ImGui::End();
}

void clampToImage(cv::Point2d &p, const cv::Mat &image) {
	p.x = std::clamp((int)p.x, 0, image.cols - 1);
	p.y = std::clamp((int)p.y, 0, image.rows - 1);
}

void generateMorph(cv::Mat sourceImage, cv::Mat targetImage) {
	cv::Mat destImage(sourceImage.rows, sourceImage.cols, sourceImage.type(), cv::Scalar(0, 0, 0));
	double a = 10, b = 1.5, p = 0.2;

	for (int col = 0; col < targetImage.cols; col++) {
		for (int row = 0; row < targetImage.rows; row++) {
			cv::Point2d destX(col, row);
			cv::Point2d dispSum(0, 0);
			double weightSum = 0;

			for (int i = 0; i < targetEdges.size(); i++) {
				cv::Point2d destP = convToPoint2d(targetEdges[i].head);
				cv::Point2d destQ = convToPoint2d(targetEdges[i].tail);
				cv::Point2d sourceP = convToPoint2d(sourceEdges[i].head);
				cv::Point2d sourceQ = convToPoint2d(sourceEdges[i].tail);

				float u = (destX - destP).dot(destQ - destP) / (cv::norm(destQ - destP) * cv::norm(destQ - destP));
				float v = (destX - destP).dot(perp(destQ - destP)) / cv::norm(destQ - destP);

				cv::Point2d sourceX = sourceP + u * (sourceQ - sourceP) + (v * perp(sourceQ - sourceP)) / cv::norm(sourceQ - sourceP);
				cv::Point2d disp = sourceX - destX;

				double dist = 0;
				if (u >= 1)
					dist = cv::norm(destX - destQ);
				else if (u <= 0)
					dist = cv::norm(destX - destP);
				else
					dist = std::abs(v);

				double weight = std::pow(std::pow(cv::norm(destQ - destP), p) / (a + dist), b);

				weightSum += weight;
				dispSum += disp * weight;
			}
			cv::Point2d finalX = destX + dispSum / weightSum;
			clampToImage(finalX, sourceImage);
			destImage.at<cv::Vec3b>(destX) = sourceImage.at<cv::Vec3b>(std::trunc(finalX.y), std::trunc(finalX.x));
		}
	}
	imwrite("./morph.png", destImage);
	reloadMorphed = true;
}

int main() {
	if (!glfwInit())
		return -1;

	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(500, 500, "image-morphing", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return -1;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	std::string sourcePathBuffer, targetPathBuffer;
	cv::Mat sourceImage, targetImage, morphedImage;

	GLuint sourceTexture, targetTexture, morphedTexture;
	glGenTextures(1, &sourceTexture);
	glGenTextures(1, &targetTexture);

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();

		loadImageWindow(sourceImage, sourcePathBuffer, sourceTexture, "source path");
		displayImage(sourceImage, sourceTexture, "source image");

		loadImageWindow(targetImage, targetPathBuffer, targetTexture, "target path");
		displayImage(targetImage, targetTexture, "target image");

		{
			ImGui::Begin("Options");

			ImGui::Text("source edges: %i", sourceEdges.size());
			ImGui::Text("target edges: %i", targetEdges.size());

			if (ImGui::Button("Morph"))
				generateMorph(sourceImage, targetImage);
			if (ImGui::Button("Load"))
				readEdges();
			if (ImGui::Button("Save"))
				writeEdges(sourcePathBuffer, targetPathBuffer);

			ImGui::End();
		}

		if (reloadMorphed == true) {
			loadImage(morphedImage, morphedTexture, "./morph.png");
			reloadMorphed = false;
		}

		{
			ImGui::Begin("Result");
			if (!morphedImage.empty()) {
				ImVec2 winSize = ImGui::GetWindowSize();
				double scaleFactor = std::min(winSize.y / morphedImage.rows, winSize.x / morphedImage.cols);
				ImGui::Image(
					reinterpret_cast<void*>(static_cast<intptr_t>(morphedTexture)),
					ImVec2(morphedImage.cols * scaleFactor, morphedImage.rows * scaleFactor)
				);
			}
			ImGui::End();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_G))
			editHandle = !editHandle;

		//Render
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}
