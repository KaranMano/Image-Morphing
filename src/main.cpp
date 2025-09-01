#include "imgui.h"
#include "utils.h"
#include <GL/glext.h>
#include <chrono>
#include <fstream>
#include <string>

#define FPS 16
void checkGLerror(){
      GLenum error = glGetError();
      while(error != GL_NO_ERROR)
      {


         switch(error){
            case(GL_NO_ERROR):
               
               break;
            case(GL_INVALID_ENUM):
               std::cout <<  ": GL_INVALID_ENUM";
               break;
            case(GL_INVALID_VALUE):
               std::cout <<  ": GL_INVALID_VALUE";
               break;
            case(GL_INVALID_OPERATION):
               std::cout <<  ": GL_INVALID_OPERATION";
               break;
            case(GL_INVALID_FRAMEBUFFER_OPERATION):
               std::cout <<  ": GL_INVALID_FRAMEBUFFER_OPERATION";
               break;
            case(GL_OUT_OF_MEMORY):
               std::cout <<  ": GL_OUT_OF_MEMORY";
               break;
            default:
               std::cout <<  ": Unknown error!";

         }
         error = glGetError();

      }
   }

bool addHandle = false, morphing = false, eraseHandle = false, useGpu = false;
int steps = 10;

std::vector<Image> morphedImages(steps);
std::vector<Texture> morphedTextures(steps);
std::vector<std::future<Image>> morphJobs;
Image sourceImage, targetImage;
std::vector<Edge> sourceEdges, targetEdges;
Texture sourceTexture, targetTexture;

int main() {
  for (auto &texture : morphedTextures) {
    texture.set(0);
  }

  if (!glfwInit())
    return -1;

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow *window = glfwCreateWindow(500, 500, "image-morphing", NULL, NULL);

  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK)
    return -1;

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.IniFilename = INI_PATH;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  std::string sourcePathBuffer = "../examples/black.jpeg",
              targetPathBuffer = "../examples/white.jpeg";

  using frames = std::chrono::duration<double, std::ratio<1, FPS>>;
  auto startTime = std::chrono::steady_clock::now();
  auto morphStart = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> morphPerf;

  std::ifstream computeShaderFile("../src/shaders/morph.compute");
  if (!computeShaderFile.is_open() || computeShaderFile.bad())
    return 1;
  std::string computeShaderString;
  char c;
  while (computeShaderFile.get(c)) {
    computeShaderString += c;
  }
  const char *computeShaderStrings[] = {computeShaderString.c_str()};

  GLuint computeProgram;
  GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(computeShader, 1, computeShaderStrings, NULL);
  glCompileShader(computeShader);
  GLint isCompiled = 0;
  glGetShaderiv(computeShader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(computeShader, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data();
    glDeleteShader(computeShader);
    return 1;
  }

  computeProgram = glCreateProgram();
  glAttachShader(computeProgram, computeShader);
  glLinkProgram(computeProgram);
  glGetProgramiv(computeProgram, GL_LINK_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);
    glGetProgramInfoLog(computeProgram, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data();
    glDeleteShader(computeShader);
    return 1;
  }

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();

    loadImageWindow(sourceImage, sourceTexture, sourceEdges, sourcePathBuffer,
                    "source path");
    displayImage(sourceImage, sourceTexture, sourceEdges, "source image",
                 addHandle, eraseHandle);

    loadImageWindow(targetImage, targetTexture, targetEdges, targetPathBuffer,
                    "target path");
    displayImage(targetImage, targetTexture, targetEdges, "target image",
                 addHandle, eraseHandle);

    {
      ImGui::Begin("Options");

      ImGui::Text("source edges: %lu", sourceEdges.size());
      ImGui::Text("target edges: %lu", targetEdges.size());

      if (ImGui::Button("Morph") ||
          morphing) { //! morphing && ImGui::Button("Morph")) {
        morphing = true;
        morphStart = std::chrono::high_resolution_clock::now();
        if (!useGpu) {
          morphJobs.clear();
          for (int step = 0; step < steps; step++)
            morphJobs.push_back(std::async(std::launch::async, [=]() {
              return generateMorph(sourceImage, targetImage, sourceEdges,
                                   targetEdges, step, steps);
            }));
        } else {
          std::vector<float> sourceEdgesData;
          std::vector<float> targetEdgesData;

          for (auto &edge : sourceEdges) {
            sourceEdgesData.push_back(edge.head.x);
            sourceEdgesData.push_back(edge.head.y);
            sourceEdgesData.push_back(edge.tail.x);
            sourceEdgesData.push_back(edge.tail.y);
          }
          for (auto &edge : targetEdges) {
            targetEdgesData.push_back(edge.head.x);
            targetEdgesData.push_back(edge.head.y);
            targetEdgesData.push_back(edge.tail.x);
            targetEdgesData.push_back(edge.tail.y);
          }
          GLuint sourceSSBO, targetSSBO;

          glGenBuffers(1, &sourceSSBO);
          glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sourceSSBO);
          glBufferData(GL_SHADER_STORAGE_BUFFER,
                       sourceEdgesData.size() * sizeof(float),
                       sourceEdgesData.data(), GL_STATIC_READ);

          glGenBuffers(1, &targetSSBO);
          glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, targetSSBO);
          glBufferData(GL_SHADER_STORAGE_BUFFER,
                       targetEdgesData.size() * sizeof(float),
                       targetEdgesData.data(), GL_STATIC_READ);

          glBindImageTexture(2, sourceTexture.get(), 0, GL_FALSE, 0,
                             GL_READ_ONLY, GL_RGBA8);
          glBindImageTexture(3, targetTexture.get(), 0, GL_FALSE, 0,
                             GL_READ_ONLY, GL_RGBA8);

          GLuint outputImageArray;
          glGenTextures(1, &outputImageArray);
          glBindTexture(GL_TEXTURE_2D_ARRAY, outputImageArray);
          glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, sourceImage.width(),
                         sourceImage.height(), steps);
						 checkGLerror();
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
                          GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
                          GL_LINEAR);
						  checkGLerror();
          glBindImageTexture(4, outputImageArray, 0, GL_TRUE, 0, GL_WRITE_ONLY,
                             GL_RGBA8);

          glUseProgram(computeProgram);
          glDispatchCompute(sourceImage.width(), sourceImage.height(), steps);
          glMemoryBarrier(GL_ALL_BARRIER_BITS);

          glUseProgram(0);
          glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
          glBindImageTexture(2, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
          glBindImageTexture(3, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
          glBindImageTexture(4, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
          checkGLerror();

          for (int step = 0; step < steps; step++) {
            Image image(sourceImage.width(), sourceImage.height(), 4);
            glGetTextureSubImage(
                outputImageArray, 0, 0, 0, step, sourceImage.width(),
                sourceImage.height(), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                sourceImage.width() * sourceImage.height() * 4 * sizeof(unsigned char), image.get());
            checkGLerror();
            morphedImages[step] = image;
          }

          glDeleteBuffers(1, &outputImageArray);
          glDeleteBuffers(1, &sourceSSBO);
          glDeleteBuffers(1, &targetSSBO);
        }
      }
      !morphing &&ImGui::Checkbox("Use Gpu?", &useGpu);

      if (ImGui::Button("Load")) {
        loadEdges(sourceEdges, "../examples/source.edges");
        loadEdges(targetEdges, "../examples/target.edges");
      }
      if (ImGui::Button("Save")) {
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
      if (useGpu) {
        for (int i = 0; i < steps; i++) {
          Texture texture;
          load(texture, morphedImages[i]);
          morphedTextures[i] = texture;
        }
      } else {
        for (int i = 0; i < morphJobs.size(); i++) {
          Image image = morphJobs[i].get();
          Texture texture;
          load(texture, image);
          morphedImages[i] = image;
          morphedTextures[i] = texture;
        }
      }
      morphPerf = std::chrono::high_resolution_clock::now() - morphStart;
	  morphing = false;
    }

    {
      ImGui::Begin("Result");
      ImGui::Text(
          "Morphing complete in %lums",
          std::chrono::duration_cast<std::chrono::milliseconds>(morphPerf)
              .count());

      auto endTime = std::chrono::steady_clock::now();
      double frame = frames(endTime - startTime).count();
      frame = (int)frame % (FPS + 1);
      int imageIndex = std::abs(frame - FPS / 2) *
                       ((double)(morphedImages.size() - 1) / (FPS / 2));
      auto imageHeight = sourceImage.height();
      auto imageWidth = sourceImage.width();

      // std::cout << frame << " " << imageIndex << std::endl;
      if (imageIndex < steps && morphedTextures[imageIndex].get() != 0) {
        ImVec2 winSize = ImGui::GetWindowSize();
        double scaleFactor =
            std::min(winSize.y / imageWidth, winSize.x / imageHeight);
        ImGui::Image(
            reinterpret_cast<void *>(
                static_cast<intptr_t>(morphedTextures[imageIndex].get())),
            ImVec2(imageWidth * scaleFactor, imageHeight * scaleFactor));
      }
      ImGui::End();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_A)) {
      addHandle = !addHandle;
      if (addHandle) {
        eraseHandle = false;
      } else {
        if (sourceEdges.size() > 0 && sourceEdges.back().drawing)
          sourceEdges.pop_back();
        if (targetEdges.size() > 0 && targetEdges.back().drawing)
          targetEdges.pop_back();
      }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
      eraseHandle = !eraseHandle;
      if (eraseHandle) {
        addHandle = false;
        if (sourceEdges.size() > 0 && sourceEdges.back().drawing)
          sourceEdges.pop_back();
        if (targetEdges.size() > 0 && targetEdges.back().drawing)
          targetEdges.pop_back();
      }
    }

    // Render
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
