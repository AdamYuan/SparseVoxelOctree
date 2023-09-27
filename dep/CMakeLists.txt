cmake_minimum_required(VERSION 3.15)
project(dep)

set(CMAKE_CXX_STANDARD 11)

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
if (CMAKE_SYSTEM_NAME STREQUAL Windows)
	add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-DNOMINMAX)
ENDIF ()

include_directories(.)

set(MYVK_TESTING OFF)
add_subdirectory(MyVK)
add_subdirectory(spdlog)

add_library(dep STATIC
		stb_image.cpp
		tiny_obj_loader.cpp
		tinyfiledialogs.c
		tinyexr.cc

		# MESH OPTIMIZER
		meshoptimizer/src/indexgenerator.cpp
		meshoptimizer/src/vcacheoptimizer.cpp
		meshoptimizer/src/overdrawoptimizer.cpp
		meshoptimizer/src/vfetchoptimizer.cpp
)

target_include_directories(dep PUBLIC . meshoptimizer/src)
target_link_libraries(dep PUBLIC glfw spdlog myvk::vulkan myvk::glfw myvk::imgui)
target_compile_definitions(dep PUBLIC VK_NO_PROTOTYPES)
