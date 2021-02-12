cmake_minimum_required(VERSION 3.15)
project(SparseVoxelOctree)

set(CMAKE_CXX_STANDARD 14)

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
	add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-DNOMINMAX)
ENDIF()

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

include_directories(
	dep
	dep/glfw/include
	dep/meshoptimizer/src
	dep/spdlog/include
	dep/volk
	)

add_subdirectory(dep/glfw)
add_subdirectory(dep/spdlog)
add_subdirectory(dep/volk)

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
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/quad_sampler.frag ${CMAKE_SOURCE_DIR}/src/spirv/quad_sampler.frag.u32)

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
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/path_tracer_viewer.frag ${CMAKE_SOURCE_DIR}/src/spirv/path_tracer_viewer.frag.u32)
ENDIF()

add_executable(SparseVoxelOctree
	# SPIRV SHADERS
	src/spirv/quad.vert.u32
	src/spirv/quad_sampler.frag.u32

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
	src/spirv/path_tracer_viewer.frag.u32

	# SINGLE HEADERS
	dep/vk_mem_alloc.cpp
	dep/vk_queue_selector.c
	dep/stb_image.cpp
	dep/tiny_obj_loader.cpp
	dep/tinyfiledialogs.c
	dep/tinyexr.cc

	# IMGUI
	dep/imgui/imgui.cpp
	dep/imgui/imgui_draw.cpp
	dep/imgui/imgui_impl_glfw.cpp
	dep/imgui/imgui_widgets.cpp
	dep/imgui/imgui_tables.cpp

	# MESH OPTIMIZER
	dep/meshoptimizer/src/indexgenerator.cpp
	dep/meshoptimizer/src/vcacheoptimizer.cpp
	dep/meshoptimizer/src/overdrawoptimizer.cpp
	dep/meshoptimizer/src/vfetchoptimizer.cpp

	# MAIN PROGRAM
	src/main.cpp
	src/QuadSpirv.hpp
	src/Config.hpp
	src/Application.cpp
	src/Application.hpp
	src/Camera.cpp
	src/Camera.hpp
	src/ImGuiRenderer.cpp
	src/ImGuiRenderer.hpp
	src/ImGuiHelper.cpp
	src/ImGuiHelper.hpp
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

	# MYVK
	src/myvk/DeviceObjectBase.hpp
	src/myvk/ImageBase.cpp
	src/myvk/ImageBase.hpp
	src/myvk/Image.cpp
	src/myvk/Image.hpp
	src/myvk/BufferBase.cpp
	src/myvk/BufferBase.hpp
	src/myvk/Buffer.cpp
	src/myvk/Buffer.hpp
	src/myvk/CommandBuffer.cpp
	src/myvk/CommandBuffer.hpp
	src/myvk/CommandPool.cpp
	src/myvk/CommandPool.hpp
	src/myvk/Device.cpp
	src/myvk/Device.hpp
	src/myvk/Instance.cpp
	src/myvk/Instance.hpp
	src/myvk/PhysicalDevice.cpp
	src/myvk/PhysicalDevice.hpp
	src/myvk/Queue.cpp
	src/myvk/Queue.hpp
	src/myvk/DeviceCreateInfo.cpp
	src/myvk/DeviceCreateInfo.hpp
	src/myvk/Surface.cpp
	src/myvk/Surface.hpp
	src/myvk/Swapchain.cpp
	src/myvk/Swapchain.hpp
	src/myvk/SwapchainImage.cpp
	src/myvk/SwapchainImage.hpp
	src/myvk/Fence.cpp
	src/myvk/Fence.hpp
	src/myvk/Semaphore.cpp
	src/myvk/Semaphore.hpp
	src/myvk/ImageView.cpp
	src/myvk/ImageView.hpp
	src/myvk/RenderPass.cpp
	src/myvk/RenderPass.hpp
	src/myvk/PipelineBase.cpp
	src/myvk/PipelineBase.hpp
	src/myvk/PipelineLayout.cpp
	src/myvk/PipelineLayout.hpp
	src/myvk/DescriptorSetLayout.cpp
	src/myvk/DescriptorSetLayout.hpp
	src/myvk/ShaderModule.cpp
	src/myvk/ShaderModule.hpp
	src/myvk/GraphicsPipeline.cpp
	src/myvk/GraphicsPipeline.hpp
	src/myvk/ComputePipeline.cpp
	src/myvk/ComputePipeline.hpp
	src/myvk/Framebuffer.cpp
	src/myvk/Framebuffer.hpp
	src/myvk/DescriptorPool.cpp
	src/myvk/DescriptorPool.hpp
	src/myvk/DescriptorSet.cpp
	src/myvk/DescriptorSet.hpp
	src/myvk/FrameManager.cpp
	src/myvk/FrameManager.hpp
	src/myvk/Sampler.cpp
	src/myvk/Sampler.hpp
	src/myvk/ObjectTracker.cpp
	src/myvk/ObjectTracker.hpp
	src/myvk/QueryPool.cpp
	src/myvk/QueryPool.hpp
	)

target_compile_definitions(SparseVoxelOctree PUBLIC VK_NO_PROTOTYPES)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(SparseVoxelOctree PRIVATE OpenMP::OpenMP_CXX)
endif()
find_package(Threads REQUIRED)
target_link_libraries(SparseVoxelOctree PRIVATE glfw spdlog volk Threads::Threads)
