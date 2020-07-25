#include "OctreeTracer.hpp"
#include "myvk/ShaderModule.hpp"
#include "QuadSpirv.hpp"
#include "OctreeTracerSpirv.hpp"
#include "Config.hpp"

void OctreeTracer::create_descriptor_pool(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	m_descriptor_pool = myvk::DescriptorPool::Create(device, frame_count, {
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, frame_count}
	});
}

void OctreeTracer::create_layouts(const std::shared_ptr<myvk::Device> &device) {
	{
		VkDescriptorSetLayoutBinding beam_image_binding = {};
		beam_image_binding.binding = 0;
		beam_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		beam_image_binding.descriptorCount = 1;
		beam_image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {beam_image_binding});
	}
	m_pipeline_layout = myvk::PipelineLayout::Create(
		device,
		{m_octree->GetDescriptorSetLayoutPtr(), m_camera->GetDescriptorSetLayout(), m_descriptor_set_layout},
		{{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 5 * sizeof(uint32_t)}});
}

void
OctreeTracer::create_frame_resources(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	m_frame_resources.resize(frame_count);

	for (auto &i : m_frame_resources) {
		i.m_beam_image = myvk::Image::CreateTexture2D(device, {kBeamWidth, kBeamHeight}, 1, VK_FORMAT_R32_SFLOAT,
													  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		i.m_beam_image_view = myvk::ImageView::Create(i.m_beam_image, VK_IMAGE_VIEW_TYPE_2D);
		i.m_beam_sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		i.m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
		i.m_descriptor_set->UpdateCombinedImageSampler(i.m_beam_sampler, i.m_beam_image_view, 0);
	}
}

void OctreeTracer::create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
												 VkExtent2D extent) {
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

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

	m_graphics_pipeline = myvk::GraphicsPipeline::Create(m_pipeline_layout, render_pass, pipeline_info);
}

void
OctreeTracer::Initialize(const Octree &octree, const Camera &camera,
						 const std::shared_ptr<myvk::RenderPass> &render_pass,
						 uint32_t subpass, uint32_t frame_count) {
	m_octree = &octree;
	m_camera = &camera;
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

	create_descriptor_pool(device, frame_count);
	create_layouts(device);
	create_frame_resources(device, frame_count);
	create_main_graphics_pipeline(render_pass, subpass, {kWidth, kHeight});
}

void OctreeTracer::CmdCompute(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const {
}

void
OctreeTracer::CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, uint32_t current_frame) const {
	const auto &cur = m_frame_resources[current_frame];
	command_buffer->CmdBindPipeline(m_graphics_pipeline);
	command_buffer->CmdBindDescriptorSets(
		{m_octree->GetDescriptorSetPtr(), m_camera->GetFrameDescriptorSet(current_frame), cur.m_descriptor_set},
		m_graphics_pipeline);
	uint32_t push_constants[] = {kWidth, kHeight, (uint32_t) m_view_type, m_beam_enable, kBeamSize};
	command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants),
									 push_constants);
	command_buffer->CmdDraw(3, 1, 0, 0);
}

