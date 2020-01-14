cmake_minimum_required(VERSION 3.7)
project(SparseVoxelOctree)

set(CMAKE_CXX_STANDARD 11)

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
	add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-DNOMINMAX)
ENDIF()


file(GLOB SHADER_FILES
	shaders/*.vert
	shaders/*.frag
	shaders/*.comp
	)
add_executable(shaders_gen src/shaders_gen.cpp)

add_custom_command(
	OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/src/ShaderSrc.hpp
	COMMAND shaders_gen ${CMAKE_CURRENT_SOURCE_DIR}/src/ShaderSrc.hpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS
	shaders_gen ${SHADER_FILES}
	)

add_subdirectory(
	dep/glfw
	)

include_directories(
	dep/gl3w/include
	dep/glfw/include
	dep
	)

file(GLOB DEP_FILES
	dep/gl3w/src/gl3w.c
	dep/mygl3/utils/*.hpp
	dep/mygl3/*.hpp
	dep/imgui/*.cpp
	dep/tinyfiledialogs/tinyfiledialogs.*
	)

find_package(OpenGL REQUIRED)

add_executable(
	SparseVoxelOctree
	${DEP_FILES}
	src/main.cpp
	src/Application.cpp
	src/Application.hpp
	src/Camera.cpp
	src/Camera.hpp
	src/Config.hpp
	src/Noise.inl
	src/OctreeBuilder.cpp
	src/OctreeBuilder.hpp
	src/Octree.cpp
	src/Octree.hpp
	src/OctreeTracer.cpp
	src/OctreeTracer.hpp
	src/OglBindings.hpp
	src/PathTracer.cpp
	src/PathTracer.hpp
	src/Scene.cpp
	src/Scene.hpp
	src/ScreenQuad.cpp
	src/ScreenQuad.hpp
	src/Sobol.cpp
	src/Sobol.hpp
	src/Sobol.inl
	src/Voxelizer.cpp
	src/Voxelizer.hpp
	src/ShaderSrc.hpp
	)
target_link_libraries(
	SparseVoxelOctree 
	${CMAKE_DL_LIBS} 
	${OPENGL_opengl_LIBRARY} 
	glfw
	${GLFW_LIBRARIES}
	)
