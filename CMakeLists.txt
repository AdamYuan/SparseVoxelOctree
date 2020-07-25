cmake_minimum_required(VERSION 3.9)
project(VulkanSVO)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()

include_directories(
		dep
		dep/glfw/include
)

add_subdirectory(dep/glfw)

function(add_spirv_shader INPUT_FILE OUTPUT_FILE)
	add_custom_command(
			OUTPUT ${OUTPUT_FILE}
			COMMAND glslc ${INPUT_FILE} -o ${OUTPUT_FILE} -O -Os
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			DEPENDS ${INPUT_FILE}
	)
endfunction()

add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/quad.vert ${CMAKE_SOURCE_DIR}/shaders/quad_vert.spv)

add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.vert ${CMAKE_SOURCE_DIR}/shaders/voxelizer_vert.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.frag ${CMAKE_SOURCE_DIR}/shaders/voxelizer_frag.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/voxelizer.geom ${CMAKE_SOURCE_DIR}/shaders/voxelizer_geom.spv)

add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tag_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_tag_node_comp.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_init_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_init_node_comp.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node.comp ${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node_comp.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg.comp ${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg_comp.spv)

add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer.frag ${CMAKE_SOURCE_DIR}/shaders/octree_tracer_frag.spv)
add_spirv_shader(${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam.frag ${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam_frag.spv)

add_executable(byte_packer src/byte_packer.cpp dep/tiny_obj_loader.cpp)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/VoxelizerSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND byte_packer src/VoxelizerSpirv.hpp
		shaders/voxelizer_vert.spv kVoxelizerVertSpv
		shaders/voxelizer_geom.spv kVoxelizerGeomSpv
		shaders/voxelizer_frag.spv kVoxelizerFragSpv

		DEPENDS byte_packer
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_vert.spv
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_geom.spv
		${CMAKE_SOURCE_DIR}/shaders/voxelizer_frag.spv
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/OctreeBuilderSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND byte_packer src/OctreeBuilderSpirv.hpp
		shaders/octree_tag_node_comp.spv kOctreeTagNodeCompSpv
		shaders/octree_init_node_comp.spv kOctreeInitNodeCompSpv
		shaders/octree_alloc_node_comp.spv kOctreeAllocNodeCompSpv
		shaders/octree_modify_arg_comp.spv kOctreeModifyArgCompSpv

		DEPENDS byte_packer
		${CMAKE_SOURCE_DIR}/shaders/octree_tag_node_comp.spv
		${CMAKE_SOURCE_DIR}/shaders/octree_init_node_comp.spv
		${CMAKE_SOURCE_DIR}/shaders/octree_alloc_node_comp.spv
		${CMAKE_SOURCE_DIR}/shaders/octree_modify_arg_comp.spv
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/QuadSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND byte_packer src/QuadSpirv.hpp
		shaders/quad_vert.spv kQuadVertSpv

		DEPENDS byte_packer
		${CMAKE_SOURCE_DIR}/shaders/quad_vert.spv
)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/OctreeTracerSpirv.hpp

		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

		COMMAND byte_packer src/OctreeTracerSpirv.hpp
		shaders/octree_tracer_frag.spv kOctreeTracerFragSpv
		shaders/octree_tracer_beam_frag.spv kOctreeTracerBeamFragSpv

		DEPENDS byte_packer
		${CMAKE_SOURCE_DIR}/shaders/octree_tracer_frag.spv
		${CMAKE_SOURCE_DIR}/shaders/octree_tracer_beam_frag.spv
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

		src/main.cpp
		src/Config.hpp
		src/Application.cpp
		src/Application.hpp
		src/Camera.cpp
		src/Camera.hpp
		src/ImGuiRenderer.cpp
		src/ImGuiRenderer.hpp
		src/Scene.cpp
		src/Scene.hpp
		src/AtomicCounter.cpp
		src/AtomicCounter.hpp
		src/OctreeBuilder.cpp
		src/OctreeBuilder.hpp
		src/Voxelizer.cpp
		src/Voxelizer.hpp
		src/OctreeTracer.cpp
		src/OctreeTracer.hpp
		src/Octree.cpp
		src/Octree.hpp

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
		)

find_package(Threads)
target_compile_definitions(VulkanSVO PUBLIC VK_NO_PROTOTYPES)
target_link_libraries(VulkanSVO ${CMAKE_DL_LIBS} glfw Threads::Threads)
