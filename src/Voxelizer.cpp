#include "Voxelizer.hpp"
#include "myvk/ShaderModule.hpp"
#include <spdlog/spdlog.h>

std::shared_ptr<Voxelizer> Voxelizer::Create(const std::shared_ptr<Scene> &scene,
                                             const std::shared_ptr<myvk::CommandPool> &command_pool,
                                             uint32_t octree_level) {
	std::shared_ptr<Voxelizer> ret = std::make_shared<Voxelizer>();

	ret->m_level = octree_level;
	ret->m_voxel_resolution = 1u << octree_level;
	ret->m_scene_ptr = scene;

	std::shared_ptr<myvk::Device> device = command_pool->GetDevicePtr();
	ret->m_atomic_counter.Initialize(device);
	ret->create_descriptors(device);
	ret->create_render_pass(device);
	ret->create_pipeline(device);
	ret->m_framebuffer =
	    myvk::Framebuffer::Create(ret->m_render_pass, {}, {ret->m_voxel_resolution, ret->m_voxel_resolution});
	ret->count_and_create_fragment_list(command_pool);

	return ret;
}

void Voxelizer::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}});
	{
		VkDescriptorSetLayoutBinding atomic_counter_binding = {};
		atomic_counter_binding.binding = 0;
		atomic_counter_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		atomic_counter_binding.descriptorCount = 1;
		atomic_counter_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding fragment_list_binding = {};
		fragment_list_binding.binding = 1;
		fragment_list_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		fragment_list_binding.descriptorCount = 1;
		fragment_list_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(
		    device, {{atomic_counter_binding, 0}, {fragment_list_binding, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT}});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
	m_descriptor_set->UpdateStorageBuffer(m_atomic_counter.GetBuffer(), 0);
}

void Voxelizer::create_render_pass(const std::shared_ptr<myvk::Device> &device) {
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pColorAttachments = nullptr;
	subpass.pDepthStencilAttachment = nullptr;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 0;
	render_pass_info.pAttachments = nullptr;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 0;
	render_pass_info.pDependencies = nullptr;

	m_render_pass = myvk::RenderPass::Create(device, render_pass_info);
}

void Voxelizer::create_pipeline(const std::shared_ptr<myvk::Device> &device) {
	constexpr uint32_t kVoxelizerVertSpv[] = {
#include "spirv/voxelizer.vert.u32"
	};
	constexpr uint32_t kVoxelizerGeomSpv[] = {
#include "spirv/voxelizer.geom.u32"
	};
	constexpr uint32_t kVoxelizerFragSpv[] = {
#include "spirv/voxelizer.frag.u32"
	};

	m_pipeline_layout =
	    myvk::PipelineLayout::Create(device, {m_descriptor_set_layout, m_scene_ptr->GetDescriptorSetLayout()},
	                                 {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t) * 4}});

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, geom_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVoxelizerVertSpv, sizeof(kVoxelizerVertSpv));
	geom_shader_module = myvk::ShaderModule::Create(device, kVoxelizerGeomSpv, sizeof(kVoxelizerGeomSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kVoxelizerFragSpv, sizeof(kVoxelizerFragSpv));

	uint32_t specialization_texture_num = std::max(m_scene_ptr->GetTextureCount(), 1u);
	VkSpecializationMapEntry frag_spec_entry = {0, 0, sizeof(uint32_t)};
	VkSpecializationInfo frag_spec_info = {1, &frag_spec_entry, sizeof(uint32_t), &specialization_texture_num};

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    geom_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_GEOMETRY_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, &frag_spec_info)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable(Scene::GetVertexBindingDescriptions(),
	                                           Scene::GetVertexAttributeDescriptions());
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_viewport_state.Enable({{0, 0, (float)m_voxel_resolution, (float)m_voxel_resolution}},
	                                       {{{0, 0}, {m_voxel_resolution, m_voxel_resolution}}});
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_NONE);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_4_BIT);

	m_pipeline = myvk::GraphicsPipeline::Create(m_pipeline_layout, m_render_pass, shader_stages, pipeline_state, 0);
}

void Voxelizer::count_and_create_fragment_list(const std::shared_ptr<myvk::CommandPool> &command_pool) {
	{
		m_atomic_counter.Reset(command_pool, 0);
		std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
		command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		command_buffer->CmdBeginRenderPass(m_render_pass, m_framebuffer, {});
		{
			command_buffer->CmdBindPipeline(m_pipeline);
			command_buffer->CmdBindDescriptorSets({m_descriptor_set, m_scene_ptr->GetDescriptorSet()}, m_pipeline, {});
			uint32_t push_constants[] = {m_voxel_resolution, 1};
			command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(uint32_t),
			                                 push_constants);
			m_scene_ptr->CmdDraw(command_buffer, m_pipeline_layout, sizeof(uint32_t) * 2);
		}
		command_buffer->CmdEndRenderPass();
		command_buffer->End();
		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
		command_buffer->Submit(fence);
		fence->Wait();
	}

	m_voxel_fragment_count = m_atomic_counter.Read(command_pool);
	m_atomic_counter.Reset(command_pool, 0);

	m_voxel_fragment_list =
	    myvk::Buffer::Create(command_pool->GetDevicePtr(), m_voxel_fragment_count * sizeof(uint32_t) * 2,
	                         VMA_MEMORY_USAGE_GPU_ONLY, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	m_descriptor_set->UpdateStorageBuffer(m_voxel_fragment_list, 1);

	spdlog::info("Voxel fragment list created with {} voxels ({} MB)", m_voxel_fragment_count,
	             m_voxel_fragment_list->GetSize() / 1000000.0);
}

void Voxelizer::CmdVoxelize(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBeginRenderPass(m_render_pass, m_framebuffer, {});
	{
		command_buffer->CmdBindPipeline(m_pipeline);
		command_buffer->CmdBindDescriptorSets({m_descriptor_set, m_scene_ptr->GetDescriptorSet()}, m_pipeline, {});

		uint32_t push_constants[] = {m_voxel_resolution, 0};
		command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(uint32_t),
		                                 push_constants);
		m_scene_ptr->CmdDraw(command_buffer, m_pipeline_layout, sizeof(uint32_t) * 2);
	}
	command_buffer->CmdEndRenderPass();
}
