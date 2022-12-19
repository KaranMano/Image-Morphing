#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include <dlib/opencv.h>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

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
#include <future>
#include <thread>
#include <chrono>

#include "pixel.h"
#include "edge.h"

#define FPS 16

bool addHandle = false, morphing = false, eraseHandle = false;
int steps = 3;
std::vector<Edge> sourceMarkers(0);
std::vector<Edge> targetMarkers(0);
std::vector<std::tuple<int, int, int>> morphedTextures;
std::vector<std::future<cv::Mat>> morphJobs(steps + 1);

inline ImVec2 conv(const cv::Point2d &p) {
	return ImVec2(p.x, p.y);
}

cv::Point2d perp(const cv::Point2d &v) {
	cv::Point3d planePerp(0, 0, 1);
	cv::Point3d p = cv::Point3d(v.x, v.y, 0).cross(planePerp);
	return cv::Point2d(p.x, p.y);
}

void writeEdges(std::string sourcePath, std::string targetPath) {
	std::ofstream sourceLog("./source.edges");
	std::ofstream targetLog("./target.edges");

	for (const auto &edge : sourceMarkers) {
		sourceLog << " " << edge;
	}

	for (const auto &edge : targetMarkers)
		targetLog << " " << edge;
}

void readEdges() {
	targetMarkers.clear();
	sourceMarkers.clear();
	std::ifstream sourceLog("./source.edges");
	std::ifstream targetLog("./target.edges");

	if (sourceLog.good()) {
		while (!sourceLog.eof()) {
			sourceMarkers.emplace_back();
			sourceLog >> sourceMarkers.back();
		}
	}

	if (targetLog.good()) {
		while (!targetLog.eof()) {
			targetMarkers.emplace_back();
			targetLog >> targetMarkers.back();
		}
	}
}

void printEdges() {
	std::cout << "sourceEdges: \n";
	for (const auto &edge : sourceMarkers)
		std::cout << edge << std::endl;

	std::cout << "targetEdges: \n";
	for (const auto &edge : targetMarkers)
		std::cout << edge << std::endl;
}

GLuint createTexture(cv::Mat &image) {
	GLuint texture;
	glGenTextures(1, &texture);
	cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
	cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
	return texture;
}

void loadImage(cv::Mat &image, GLuint &texture, const std::string &path) {
	image = cv::imread(path, cv::IMREAD_COLOR);
	if (!image.empty())
		texture = createTexture(image);
}

void loadImageWindow(cv::Mat &image, std::string &buffer, GLuint &texture, const std::string &title) {
	ImGui::Begin(title.c_str());
	ImGui::InputText(("input##" + title).c_str(), &buffer);

	if (ImGui::Button(("submit##" + title).c_str()))
		loadImage(image, texture, buffer);
	if (ImGui::Button(("clear##" + title).c_str()))
		(title == "source path") ? sourceMarkers.clear() : targetMarkers.clear();
	ImGui::End();
}

void displayImage(cv::Mat image, GLuint texture, std::string title) {
	std::vector<Edge> *edges = nullptr;
	edges = (title == "source image") ? &sourceMarkers : &targetMarkers;

	ImGui::Begin(title.c_str());
	if (!image.empty()) {
		ImVec2 winSize = ImGui::GetWindowSize();	// store in variable to ensure usage of same window size
		double scaleFactor = std::min(winSize.y / image.rows, winSize.x / image.cols);
		ImGui::Image(
			reinterpret_cast<void*>(static_cast<intptr_t>(texture)),
			ImVec2(image.cols * scaleFactor, image.rows * scaleFactor)
		);
		Pixel topLeft = ImGui::GetItemRectMin(), bottomRight = ImGui::GetItemRectMax();

		if (addHandle) {
			ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, IM_COL32(0, 255, 0, 255), 0, 0, 5);
			if (ImGui::IsItemClicked()) {
				Pixel mousePos = ImGui::GetMousePos();
				Pixel clickPos = (mousePos - topLeft) / scaleFactor;
				if (edges->size() > 0 && edges->back().drawing)
					edges->back().setHead(clickPos);
				else {
					if (sourceMarkers.size() > 0 && sourceMarkers.back().drawing)
						sourceMarkers.pop_back();
					if (targetMarkers.size() > 0 && targetMarkers.back().drawing)
						targetMarkers.pop_back();
					edges->emplace_back(clickPos);
				}
			}
		}

		if (eraseHandle) {
			ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, IM_COL32(255, 0, 0, 255), 0, 0, 5);
		}

		for (auto &edge : *edges) {
			Pixel tail = edge.tail * scaleFactor + topLeft;
			Pixel head = (edge.drawing) ? Pixel(ImGui::GetMousePos()) : edge.head * scaleFactor + topLeft;

			int height = 10, width = 5;
			cv::Point2d headPoint = head;
			cv::Point2d tailPoint = tail;
			cv::Point2d line = (tailPoint - headPoint) / cv::norm(headPoint - tailPoint);
			cv::Point2d perpLine = perp(line);

			ImGui::GetWindowDrawList()->AddTriangleFilled(	// arrowhead
				conv(headPoint),
				conv(headPoint + line * height + perpLine * width),
				conv(headPoint + line * height - perpLine * width),
				IM_COL32(255, 0, 0, 255));

			ImGui::GetWindowDrawList()->AddLine(tail, conv(headPoint + line * 10), IM_COL32(255, 0, 0, 255), 2);
		}

		if (eraseHandle) {
			for (auto it = edges->begin(); it != edges->end();) {
				Edge &edge = *it;
				Pixel tail = edge.tail * scaleFactor + topLeft;
				Pixel head = edge.head * scaleFactor + topLeft;

				int height = 10, width = 5;
				cv::Point2d headPoint = head;
				cv::Point2d tailPoint = tail;
				cv::Point2d line = (tailPoint - headPoint) / cv::norm(headPoint - tailPoint);
				cv::Point2d perpLine = perp(line);

				if (ImGui::IsMouseClicked(0)) {
					Pixel clickPos = ImGui::GetMousePos();
					cv::Point2d mousePoint = clickPos;
					if (std::abs((mousePoint - tailPoint).dot(line)) < cv::norm(headPoint - tailPoint)
						&& std::abs((mousePoint - tailPoint).dot(perpLine)) < width) {
						if (title == "source image")
							targetMarkers.erase(targetMarkers.begin() + (int)(it - edges->begin()));
						else							
							sourceMarkers.erase(sourceMarkers.begin() + (int)(it - edges->begin()));
						it = edges->erase(it);
						std::cout << "erased\n";
					}
					else {
						it++;
					}
				}
				else {
					it++;
				}
			}
		}
	}
	ImGui::End();
}

void clampToImage(cv::Point2d &p, const cv::Mat &image) {
	p.x = std::clamp((int)p.x, 0, image.cols - 1);
	p.y = std::clamp((int)p.y, 0, image.rows - 1);
}

inline cv::Point2d generateIntermediatePoint(const cv::Point2d &source, const cv::Point2d &dest, const double &step) {
	return (dest - source) * step + source;
}

cv::Mat morphImage(const cv::Mat &sourceImage,
	const cv::Mat &targetImage,
	const std::vector<Edge> sourceEdges,
	const std::vector<Edge> targetEdges,
	const double &step)
{
	cv::Mat destImage(sourceImage.rows, sourceImage.cols, sourceImage.type(), cv::Scalar(0, 0, 0));
	double a = 10, b = 1.5, p = 0.2;
	//! gpu optimisation maybe
	for (int col = 0; col < targetImage.cols; col++) {
		for (int row = 0; row < targetImage.rows; row++) {
			cv::Point2d destX(col, row);
			cv::Point2d dispSum(0, 0);
			double weightSum = 0;

			for (int i = 0; i < targetEdges.size(); i++) {
				cv::Point2d destP = targetEdges[i].head;
				cv::Point2d destQ = targetEdges[i].tail;
				cv::Point2d sourceP = sourceEdges[i].head;
				cv::Point2d sourceQ = sourceEdges[i].tail;

				destP = generateIntermediatePoint(sourceP, destP, step);
				destQ = generateIntermediatePoint(sourceQ, destQ, step);

				float u = (destX - destP).dot(destQ - destP) / (cv::norm(destQ - destP) * cv::norm(destQ - destP));
				float v = (destX - destP).dot(perp(destQ - destP)) / cv::norm(destQ - destP);

				cv::Point2d sourceX = sourceP + u * (sourceQ - sourceP) + (v * perp(sourceQ - sourceP)) / cv::norm(sourceQ - sourceP);
				cv::Point2d disp = sourceX - destX;

				double dist;
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
	return destImage;
}

cv::Mat generateMorph(const cv::Mat &sourceImage, const cv::Mat &targetImage, const int &step) {
	cv::Mat finalMorph;
	if (step == steps)
		finalMorph = targetImage;
	else if (step == 0)
		finalMorph = sourceImage;
	else {
		double alpha = (double)step / steps;
		cv::Mat sourceMorph = morphImage(targetImage, sourceImage, targetMarkers, sourceMarkers, 1 - alpha);
		cv::Mat destMorph = morphImage(sourceImage, targetImage, sourceMarkers, targetMarkers, alpha);
		finalMorph = (alpha)* sourceMorph + (1 - alpha)* destMorph;
	}
	cv::imwrite("morph" + std::to_string(step) + ".png", finalMorph);
	return finalMorph;
}

void generateFeatures(cv::Mat &image, std::vector<Edge> &markers, int index) {
	markers.clear();
	std::cout << "generating \n";
	cv::Mat image2 = image.clone();
	
	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
	
	dlib::shape_predictor shape_model;
	dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> shape_model;

	dlib::array2d<dlib::bgr_pixel> dlib_image;
	dlib::assign_image(dlib_image, dlib::cv_image<dlib::bgr_pixel>(image2));
	std::pair<int, int> oldSize = std::make_pair(dlib_image.nc(), dlib_image.nr());
	dlib::pyramid_up(dlib_image);
	std::pair<int, int> newSize = std::make_pair(dlib_image.nc(), dlib_image.nr());

	std::vector<dlib::rectangle> detected_faces = detector(dlib_image);
	
	int number_of_detected_faces = detected_faces.size();
	std::cout << "Number of detected faces : " << number_of_detected_faces << std::endl;

	std::vector<dlib::full_object_detection> shapes;
	int rect_x, rect_y, rect_w, rect_h;
	//dlib::image_window win;
	std::cout << "prev size =" << oldSize.first << " " << oldSize.second 
		<< "newSize = " << newSize.first << " " << newSize.second << std::endl;
	for (int i = 0; i < number_of_detected_faces; i++) {
		double scaleFactor = std::min((double)oldSize.first / newSize.first, (double)oldSize.second / oldSize.second);
		std::cout << "scale factor =" << scaleFactor << std::endl;
		dlib::full_object_detection shape = shape_model(dlib_image, detected_faces[i]);
		shapes.push_back(shape);

		std::cout << "number of parts = " << shape.num_parts() << std::endl;
		rect_x = detected_faces[i].left();
		rect_y = detected_faces[i].top();
		rect_w = detected_faces[i].right() - rect_x;
		rect_h = detected_faces[i].bottom() - rect_y;
		std::cout << "face " << rect_x << " " << rect_y << " " << rect_w << " " << rect_h << std::endl;
		cv::Rect face_rectangle(rect_x*scaleFactor, rect_y*scaleFactor, rect_w*scaleFactor, rect_h*scaleFactor);

		cv::rectangle(image2, face_rectangle, cv::Scalar(0, 255, 0), 3, 8, 0);

		for (int j = 0; j < 68; j++) {
			std::cout << "point " << shape.part(j).x() * scaleFactor << " " << shape.part(j).y() * scaleFactor << std::endl;
			cv::circle(image2, cv::Point(shape.part(j).x() * scaleFactor, shape.part(j).y() * scaleFactor), 2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
		}
		for (int j = 0; j < 68; j+=2) {
			markers.emplace_back(Pixel(shape.part(j).x() * scaleFactor, shape.part(j).y() * scaleFactor),
					Pixel(shape.part(j+1).x() * scaleFactor, shape.part(j+1).y() * scaleFactor)
				,false);
		}
	}
	cv::imwrite("feature" + std::to_string(index) + ".png", image2);
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

	std::string sourcePathBuffer = "black.jpeg", targetPathBuffer = "white.jpeg";
	cv::Mat sourceImage, targetImage, morphedImage;
	using frames = std::chrono::duration<double, std::ratio<1, FPS>>;
	auto startTime = std::chrono::steady_clock::now();

	GLuint sourceTexture, targetTexture, morphedTexture;
	int morphPerf = 0;
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

			ImGui::Text("source edges: %i", sourceMarkers.size());
			ImGui::Text("target edges: %i", targetMarkers.size());

			if (!morphing && ImGui::Button("Morph")) {
				int start = std::time(nullptr);
				morphing = true;
				morphedTextures.clear();

				for (int i = 0; i < steps + 1; i++)
					morphedTextures.emplace_back(-1, 0, 0);
				for (int step = 0; step <= steps; step++)
					morphJobs[step] = std::async(std::launch::async, generateMorph, sourceImage, targetImage, step);
				morphPerf = std::time(nullptr) - start;
			}

			if (ImGui::Button("Load"))
				readEdges();
			if (ImGui::Button("Save"))
				writeEdges(sourcePathBuffer, targetPathBuffer);
			if (ImGui::Button("generate features")) {
				generateFeatures(sourceImage, sourceMarkers, 0);
				generateFeatures(targetImage, targetMarkers, 1);
			}

			ImGui::End();
		}

		int ready = 0;
		if (morphing == true) {
			for (int i = 0; i < morphJobs.size(); i++) {
				if (!morphJobs[i].valid()) {
					ready++;
					continue;
				}
				auto status = morphJobs[i].wait_for(std::chrono::seconds(0));
				if (status == std::future_status::ready) {
					ready++;
					cv::Mat image = morphJobs[i].get();
					morphedTextures[i] = std::make_tuple(createTexture(image), image.cols, image.rows);
					morphJobs[i] = std::future<cv::Mat>();
				}
			}
			if (ready == steps + 1)
				morphing = false;
		}

		{
			ImGui::Begin("Result");
			if (morphPerf)
				ImGui::Text("Morphing complete in %i", morphPerf);

			auto endTime = std::chrono::steady_clock::now();
			double frame = frames(endTime - startTime).count();
			frame = (int)frame % (FPS + 1);
			int imageIndex = std::abs(frame - FPS / 2) * ((double)(morphedTextures.size() - 1) / (FPS / 2));

			//std::cout << frame << " " << imageIndex << std::endl;
			if (imageIndex < morphedTextures.size() && std::get<0>(morphedTextures[imageIndex]) != -1) {
				ImVec2 winSize = ImGui::GetWindowSize();
				double scaleFactor = std::min(winSize.y / std::get<2>(morphedTextures[imageIndex]), winSize.x / std::get<1>(morphedTextures[imageIndex]));
				ImGui::Image(
					reinterpret_cast<void*>(static_cast<intptr_t>(std::get<0>(morphedTextures[imageIndex]))),
					ImVec2(std::get<1>(morphedTextures[imageIndex]) * scaleFactor, std::get<2>(morphedTextures[imageIndex]) * scaleFactor)
				);
			}
			ImGui::End();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_A)) {
			addHandle = !addHandle;
			if (addHandle) {
				eraseHandle = false;
			}
			else {
				if (sourceMarkers.size() > 0 && sourceMarkers.back().drawing)
					sourceMarkers.pop_back();
				if (targetMarkers.size() > 0 && targetMarkers.back().drawing)
					targetMarkers.pop_back();
			}
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E)) {
			eraseHandle = !eraseHandle;
			if (eraseHandle)
			{
				addHandle = false;
				if (sourceMarkers.size() > 0 && sourceMarkers.back().drawing)
					sourceMarkers.pop_back();
				if (targetMarkers.size() > 0 && targetMarkers.back().drawing)
					targetMarkers.pop_back();
			}
		}

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
