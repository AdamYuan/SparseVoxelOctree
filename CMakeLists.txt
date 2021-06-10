cmake_minimum_required(VERSION 3.15)
project(SparseVoxelOctree)

set(CMAKE_CXX_STANDARD 14)

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

find_program(GLSLC "glslc")
IF(GLSLC)
	function(add_spirv_shader INPUT_FILE OUTPUT_FILE)
		add_custom_command(
			OUTPUT ${OUTPUT_FILE}
			COMMAND glslc ${INPUT_FILE} -o ${OUTPUT_FILE} -O -Os -mfmt=num
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			DEPENDS ${INPUT_FILE}
			)
	endfunction()

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/quad.vert ${CMAKE_SOURCE_DIR}/src/spirv/quad.vert.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/sobol.comp ${CMAKE_SOURCE_DIR}/src/spirv/sobol.comp.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.vert ${CMAKE_SOURCE_DIR}/src/spirv/voxelizer.vert.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.frag ${CMAKE_SOURCE_DIR}/src/spirv/voxelizer.frag.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.geom ${CMAKE_SOURCE_DIR}/src/spirv/voxelizer.geom.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tag_node.comp ${CMAKE_SOURCE_DIR}/src/spirv/octree_tag_node.comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_init_node.comp ${CMAKE_SOURCE_DIR}/src/spirv/octree_init_node.comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node.comp ${CMAKE_SOURCE_DIR}/src/spirv/octree_alloc_node.comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg.comp ${CMAKE_SOURCE_DIR}/src/spirv/octree_modify_arg.comp.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer.frag ${CMAKE_SOURCE_DIR}/src/spirv/octree_tracer.frag.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam.frag ${CMAKE_SOURCE_DIR}/src/spirv/octree_tracer_beam.frag.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/path_tracer.comp ${CMAKE_SOURCE_DIR}/src/spirv/path_tracer.comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/path_tracer_viewer_gen.frag ${CMAKE_SOURCE_DIR}/src/spirv/path_tracer_viewer_gen.frag.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/path_tracer_viewer_main.vert ${CMAKE_SOURCE_DIR}/src/spirv/path_tracer_viewer_main.vert.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/path_tracer_viewer_main.frag ${CMAKE_SOURCE_DIR}/src/spirv/path_tracer_viewer_main.frag.u32)
ENDIF()

add_executable(SparseVoxelOctree
	# SPIRV SHADERS
	src/spirv/quad.vert.u32

	src/spirv/sobol.comp.u32

	src/spirv/voxelizer.vert.u32
	src/spirv/voxelizer.frag.u32
	src/spirv/voxelizer.geom.u32

	src/spirv/octree_tag_node.comp.u32
	src/spirv/octree_init_node.comp.u32
	src/spirv/octree_alloc_node.comp.u32
	src/spirv/octree_modify_arg.comp.u32

	src/spirv/octree_tracer.frag.u32
	src/spirv/octree_tracer_beam.frag.u32

	src/spirv/path_tracer.comp.u32
	src/spirv/path_tracer_viewer_gen.frag.u32
	src/spirv/path_tracer_viewer_main.vert.u32
	src/spirv/path_tracer_viewer_main.frag.u32

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
target_link_libraries(SparseVoxelOctree PRIVATE dep Threads::Threads)

install(TARGETS SparseVoxelOctree RUNTIME DESTINATION)
