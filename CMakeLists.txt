cmake_minimum_required(VERSION 3.9)
project(VulkanSVO)

set(CMAKE_CXX_STANDARD 14)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

include_directories(
		dep
		dep/glfw/include
		dep/meshoptimizer/src
		dep/spdlog/include
)

add_subdirectory(dep/glfw)
add_subdirectory(dep/spdlog)

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

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/quad.vert ${CMAKE_SOURCE_DIR}/shaders/quad_vert.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.vert ${CMAKE_SOURCE_DIR}/shaders/voxelizer_vert.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.frag ${CMAKE_SOURCE_DIR}/shaders/voxelizer_frag.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.geom ${CMAKE_SOURCE_DIR}/shaders/voxelizer_geom.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tag_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_tag_node_comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_init_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_init_node_comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node_comp.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg.comp ${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg_comp.u32)

	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer.frag ${CMAKE_SOURCE_DIR}/shaders/octree_tracer_frag.u32)
	add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam.frag ${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam_frag.u32)
ENDIF()

add_executable(spirv_u32_packer src/spirv_u32_packer.cpp dep/tiny_obj_loader.cpp)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/VoxelizerSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND spirv_u32_packer src/VoxelizerSpirv.hpp
		shaders/voxelizer_vert.u32 kVoxelizerVertSpv
		shaders/voxelizer_geom.u32 kVoxelizerGeomSpv
		shaders/voxelizer_frag.u32 kVoxelizerFragSpv

		DEPENDS spirv_u32_packer
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_vert.u32
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_geom.u32
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_frag.u32
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/OctreeBuilderSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND spirv_u32_packer src/OctreeBuilderSpirv.hpp
		shaders/octree_tag_node_comp.u32 kOctreeTagNodeCompSpv
		shaders/octree_init_node_comp.u32 kOctreeInitNodeCompSpv
		shaders/octree_alloc_node_comp.u32 kOctreeAllocNodeCompSpv
		shaders/octree_modify_arg_comp.u32 kOctreeModifyArgCompSpv

		DEPENDS spirv_u32_packer
		${CMAKE_SOURCE_DIR}/shaders/octree_tag_node_comp.u32
		${CMAKE_SOURCE_DIR}/shaders/octree_init_node_comp.u32
		${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node_comp.u32
		${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg_comp.u32
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/QuadSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND spirv_u32_packer src/QuadSpirv.hpp
		shaders/quad_vert.u32 kQuadVertSpv

		DEPENDS spirv_u32_packer
		${CMAKE_SOURCE_DIR}/shaders/quad_vert.u32
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/OctreeTracerSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND spirv_u32_packer src/OctreeTracerSpirv.hpp
		shaders/octree_tracer_frag.u32 kOctreeTracerFragSpv
		shaders/octree_tracer_beam_frag.u32 kOctreeTracerBeamFragSpv

		DEPENDS spirv_u32_packer
		${CMAKE_SOURCE_DIR}/shaders/octree_tracer_frag.u32
		${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam_frag.u32
)

add_executable(VulkanSVO
		dep/vk_mem_alloc.cpp
		dep/vk_queue_selector.c
		dep/stb_image.cpp
		dep/tiny_obj_loader.cpp
		dep/tinyfiledialogs.c
		dep/volk.c

		dep/imgui/imgui.cpp
		dep/imgui/imgui_draw.cpp
		dep/imgui/imgui_impl_glfw.cpp
		dep/imgui/imgui_widgets.cpp

		dep/meshoptimizer/src/indexgenerator.cpp
		dep/meshoptimizer/src/vcacheoptimizer.cpp
		dep/meshoptimizer/src/overdrawoptimizer.cpp
		dep/meshoptimizer/src/vfetchoptimizer.cpp

		src/main.cpp
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

		src/VoxelizerSpirv.hpp
		src/OctreeBuilderSpirv.hpp
		src/QuadSpirv.hpp
		src/OctreeTracerSpirv.hpp

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

find_package(Threads)
target_compile_definitions(VulkanSVO PUBLIC VK_NO_PROTOTYPES)
target_link_libraries(VulkanSVO ${CMAKE_DL_LIBS} glfw spdlog Threads::Threads)
