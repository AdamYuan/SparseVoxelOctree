cmake_minimum_required(VERSION 3.15)
project(SparseVoxelOctree)

set(CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-DNOMINMAX)
ENDIF()

# Hide the console window in visual studio projects - Release
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_subdirectory(dep)
add_subdirectory(shader)

add_executable(SparseVoxelOctree
	src/main.cpp
	src/QuadSpirv.hpp
	src/Config.hpp
	src/Application.cpp
	src/Application.hpp
	src/Camera.cpp
	src/Camera.hpp
	src/ImGuiRenderer.cpp
	src/ImGuiRenderer.hpp
	src/Scene.cpp
	src/Scene.hpp
	src/Counter.cpp
	src/Counter.hpp
	src/OctreeBuilder.cpp
	src/OctreeBuilder.hpp
	src/Voxelizer.cpp
	src/Voxelizer.hpp
	src/OctreeTracer.cpp
	src/OctreeTracer.hpp
	src/Octree.cpp
	src/Octree.hpp
	src/Sobol.cpp
	src/Sobol.hpp
	src/PathTracer.cpp
	src/PathTracer.hpp
	src/PathTracerViewer.cpp
	src/PathTracerViewer.hpp
	src/LoaderThread.cpp
	src/LoaderThread.hpp
	src/PathTracerThread.cpp
	src/PathTracerThread.hpp
	src/UIHelper.cpp
	src/UIHelper.hpp
	src/UILog.cpp
	src/UILog.hpp
	src/UILoader.cpp
	src/UILoader.hpp
	src/UIPathTracer.cpp
	src/UIPathTracer.hpp
	src/UIOctreeTracer.cpp
	src/UIOctreeTracer.hpp
	)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(SparseVoxelOctree PRIVATE OpenMP::OpenMP_CXX)
endif()
find_package(Threads REQUIRED)
target_link_libraries(SparseVoxelOctree PRIVATE dep shader Threads::Threads)

install(TARGETS SparseVoxelOctree RUNTIME DESTINATION)
