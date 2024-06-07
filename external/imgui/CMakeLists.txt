cmake_minimum_required(VERSION 3.16)

file(GLOB LIB_FILES *)
add_library(imgui STATIC ${LIB_FILES})
target_link_libraries(imgui 
	PRIVATE glfw
)
