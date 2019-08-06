cmake_minimum_required(VERSION 3.7)
project(SparseVoxelOctree)

set(CMAKE_CXX_STANDARD 11)

include_directories(
		dep/gl3w/include
		dep
)

file(GLOB SOURCE_FILES
		dep/gl3w/src/gl3w.c
		dep/mygl3/*.hpp
		dep/imgui/*.cpp
		src/*.*)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(SparseVoxelOctree ${SOURCE_FILES})
target_link_libraries(SparseVoxelOctree ${CMAKE_DL_LIBS} ${OPENGL_opengl_LIBRARY} glfw)
