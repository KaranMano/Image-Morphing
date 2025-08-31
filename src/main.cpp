#include "utils.h"

#define FPS 16

bool addHandle = false, morphing = false, eraseHandle = false;
int steps = 3;

std::vector<Image> morphedImages(steps + 1);
std::vector<Texture> morphedTextures(steps + 1);
std::vector<std::future<Image>> morphJobs;
Image sourceImage, targetImage;
std::vector<Edge> sourceEdges, targetEdges;
Texture sourceTexture, targetTexture;

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
	io.IniFilename = INI_PATH;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	std::string sourcePathBuffer = "../examples/black.jpeg", targetPathBuffer = "../examples/white.jpeg";
	
	using frames = std::chrono::duration<double, std::ratio<1, FPS>>;
	auto startTime = std::chrono::steady_clock::now();

	int morphPerf = 0;
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();

		loadImageWindow(sourceImage, sourceTexture, sourceEdges, sourcePathBuffer, "source path");
		displayImage(sourceImage, sourceTexture, sourceEdges, "source image", addHandle, eraseHandle);

		loadImageWindow(targetImage, targetTexture, targetEdges, targetPathBuffer, "target path");
		displayImage(targetImage, targetTexture, targetEdges, "target image", addHandle, eraseHandle);

		{
			ImGui::Begin("Options");

			ImGui::Text("source edges: %lu", sourceEdges.size());
			ImGui::Text("target edges: %lu", targetEdges.size());

			if (!morphing && ImGui::Button("Morph")) {
				int start = std::time(nullptr);
				morphing = true;
				morphJobs.clear();
				for (int step = 0; step <= steps; step++)
					morphJobs.push_back(
						std::async(
							std::launch::async,
							[=]() {
								return generateMorph(sourceImage, targetImage, sourceEdges, targetEdges, step, steps); 
							}
						));
				morphPerf = std::time(nullptr) - start;
			}

			if (ImGui::Button("Load")){
				loadEdges(sourceEdges, "../examples/source.edges");
				loadEdges(targetEdges, "../examples/target.edges");
			}
			if (ImGui::Button("Save")){
				saveEdges(sourceEdges, "../examples/source.edges");
				saveEdges(targetEdges, "../examples/target.edges");
			}
			// if (ImGui::Button("generate features")) {
			// 	generateFeatures(sourceImage, sourceMarkers, 0);
			// 	generateFeatures(targetImage, targetMarkers, 1);
			// }

			ImGui::End();
		}

		if (morphing == true) {
			for (int i = 0; i < morphJobs.size(); i++) {
				Image image = morphJobs[i].get();
				Texture texture;
				load(texture, image);
				morphedImages[i] = image;
				morphedTextures[i] = texture;
			}
			morphing = false;
		}

		{
			ImGui::Begin("Result");
			if (morphPerf)
				ImGui::Text("Morphing complete in %i", morphPerf);

			auto endTime = std::chrono::steady_clock::now();
			double frame = frames(endTime - startTime).count();
			frame = (int)frame % (FPS + 1);
			int imageIndex = std::abs(frame - FPS / 2) * ((double)(morphedImages.size() - 1) / (FPS / 2));

			//std::cout << frame << " " << imageIndex << std::endl;
			if (imageIndex < morphedImages.size() && morphedTextures[imageIndex].get() != -1) {
				ImVec2 winSize = ImGui::GetWindowSize();
				double scaleFactor = std::min(winSize.y / morphedImages[imageIndex].width(), winSize.x / morphedImages[imageIndex].height());
				ImGui::Image(
					reinterpret_cast<void*>(static_cast<intptr_t>(morphedTextures[imageIndex].get())),
					ImVec2(morphedImages[imageIndex].width() * scaleFactor, morphedImages[imageIndex].height() * scaleFactor)
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
				if (sourceEdges.size() > 0 && sourceEdges.back().drawing)
					sourceEdges.pop_back();
				if (targetEdges.size() > 0 && targetEdges.back().drawing)
					targetEdges.pop_back();
			}
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E)) {
			eraseHandle = !eraseHandle;
			if (eraseHandle)
			{
				addHandle = false;
				if (sourceEdges.size() > 0 && sourceEdges.back().drawing)
					sourceEdges.pop_back();
				if (targetEdges.size() > 0 && targetEdges.back().drawing)
					targetEdges.pop_back();
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
