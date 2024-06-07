cmake_minimum_required( VERSION 3.16 )
project( image-morphing 
	VERSION 0.2.0
	DESCRIPTION "Image morphing using the Baier-Neely algorithm along with auto correspondence."
	LANGUAGES CXX 
)

find_package( OpenGL REQUIRED )
message(opengl?="${OPENGL_FOUND}")
if (OPENGL_FOUND)
	message(opengl_include_dir="${OPENGL_INCLUDE_DIR}")
	message(opengl_libraries="${OPENGL_LIBRARIES}")
endif()

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
add_definitions( -DGLEW_STATIC )

add_subdirectory( external/glfw )
add_subdirectory( external/glew )
add_subdirectory( external/imgui )
add_subdirectory( external/stb )

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

set( INI_PATH \"${CMAKE_SOURCE_DIR}/imgui.ini\")
configure_file(config.h.in config.h)

file( GLOB_RECURSE SOURCE_FILES src/* )

add_executable( ${CMAKE_PROJECT_NAME} WIN32 ${SOURCE_FILES} )
set_property( TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17 )
target_link_libraries( ${CMAKE_PROJECT_NAME}
	PRIVATE glfw
	PRIVATE ${OPENGL_LIBRARIES}
	PRIVATE libglew_static
	PRIVATE imgui
	PRIVATE stb
)
target_include_directories( ${CMAKE_PROJECT_NAME}
	PRIVATE ${CMAKE_BINARY_DIR}
	PRIVATE ${OPENGL_INCLUDE_DIR}
	PRIVATE external/imgui
	PRIVATE external/stb
)