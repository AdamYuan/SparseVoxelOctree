#include "OctreeTracer.hpp"
#include "myvk/ShaderModule.hpp"
#include "QuadSpirv.hpp"
#include "OctreeTracerSpirv.hpp"
#include "Config.hpp"

void OctreeTracer::create_uniform_buffers(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	m_camera_uniform_buffers.resize(frame_count);
	for (uint32_t i = 0; i < frame_count; ++i) {
		m_camera_uniform_buffers[i] = myvk::Buffer::Create(device, sizeof(Camera::Buffer), VMA_MEMORY_USAGE_CPU_TO_GPU,
														   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}
}

void OctreeTracer::create_descriptors(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	{
		VkDescriptorSetLayoutBinding octree_binding = {};
		octree_binding.binding = 0;
		octree_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octree_binding.descriptorCount = 1;
		octree_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding camera_binding = {};
		camera_binding.binding = 1;
		camera_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		camera_binding.descriptorCount = 1;
		camera_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {octree_binding, camera_binding});
	}
	{
		VkDescriptorPoolSize pool_sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frame_count},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, frame_count}
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = frame_count;
		pool_info.poolSizeCount = 2;
		pool_info.pPoolSizes = pool_sizes;

		m_descriptor_pool = myvk::DescriptorPool::Create(device, pool_info);
	}
	m_descriptor_sets = myvk::DescriptorSet::CreateMultiple(
		m_descriptor_pool,
		std::vector<std::shared_ptr<myvk::DescriptorSetLayout>>(frame_count, m_descriptor_set_layout));

	for (uint32_t i = 0; i < frame_count; ++i)
		m_descriptor_sets[i]->UpdateUniformBuffer(m_camera_uniform_buffers[i], 1);
}

void OctreeTracer::create_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
								   VkExtent2D extent) {
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

	m_pipeline_layout = myvk::PipelineLayout::Create(
		device,
		{m_descriptor_set_layout},
		{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 5 * sizeof(uint32_t)}});


	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, (uint32_t *) kQuadVertSpv, sizeof(kQuadVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, (uint32_t *) kOctreeTracerFragSpv,
													sizeof(kOctreeTracerFragSpv));

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineVertexInputStateCreateInfo vertex_input = {};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 0.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend = {};
	color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend.attachmentCount = 1;
	color_blend.pAttachments = &color_blend_attachment;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = nullptr;
	pipeline_info.pColorBlendState = &color_blend;
	pipeline_info.pDynamicState = nullptr;
	pipeline_info.subpass = subpass;

	m_pipeline = myvk::GraphicsPipeline::Create(m_pipeline_layout, render_pass, pipeline_info);
}

void OctreeTracer::Initialize(const std::shared_ptr<myvk::Swapchain> &swapchain,
							  const std::shared_ptr<myvk::RenderPass> &render_pass,
							  uint32_t subpass, uint32_t frame_count) {
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

	create_uniform_buffers(device, frame_count);
	create_descriptors(device, frame_count);
	create_pipeline(render_pass, subpass, swapchain->GetExtent());
}

void OctreeTracer::UpdateOctree(const Octree &octree) {
	for (auto &i : m_descriptor_sets) i->UpdateStorageBuffer(octree.GetBufferPtr(), 0);
}

void OctreeTracer::CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, const Camera &camera,
								   uint32_t current_frame) const {
	m_camera_uniform_buffers[current_frame]->UpdateData(camera.GetBuffer());
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({m_descriptor_sets[current_frame]}, m_pipeline, {});
	uint32_t push_constants[] = {kWidth, kHeight, (uint32_t) m_view_type, 0, 0};
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants),
									 push_constants);
	command_buffer->CmdDraw(3, 1, 0, 0);
}

