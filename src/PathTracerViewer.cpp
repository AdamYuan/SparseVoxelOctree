#include "Config.hpp"
#include "PathTracerViewer.hpp"
#include "QuadSpirv.hpp"

void PathTracerViewer::create_render_pass(const std::shared_ptr<myvk::Device> &device) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = nullptr;

	VkSubpassDependency subpass_dependency = {};
	subpass_dependency.srcSubpass = 0;
	subpass_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &subpass_dependency;

	m_gen_render_pass = myvk::RenderPass::Create(device, render_pass_info);
}

void PathTracerViewer::create_gen_graphics_pipeline(const std::shared_ptr<myvk::Device> &device) {
	m_gen_pipeline_layout = myvk::PipelineLayout::Create(device, {m_path_tracer_ptr->GetTargetDescriptorSetLayout()},
	                                                     {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t)}});
	constexpr uint32_t kPathTracerViewerGenFragSpirv[] = {
#include "spirv/path_tracer_viewer_gen.frag.u32"
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kQuadVertSpv, sizeof(kQuadVertSpv));
	frag_shader_module =
	    myvk::ShaderModule::Create(device, kPathTracerViewerGenFragSpirv, sizeof(kPathTracerViewerGenFragSpirv));

	VkPipelineShaderStageCreateInfo shader_stages[] = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	VkPipelineVertexInputStateCreateInfo vertex_input = {};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

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
	color_blend_attachment.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend = {};
	color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend.attachmentCount = 1;
	color_blend.pAttachments = &color_blend_attachment;

	VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

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
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.subpass = 0;

	m_gen_graphics_pipeline = myvk::GraphicsPipeline::Create(m_gen_pipeline_layout, m_gen_render_pass, pipeline_info);
}

void PathTracerViewer::create_descriptors(const std::shared_ptr<myvk::Device> &device) {
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}});
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {binding});
	}
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
	m_descriptor_set->UpdateCombinedImageSampler(m_sampler, m_image_view, 0);
}

void PathTracerViewer::create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass,
                                                     uint32_t subpass) {
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

	m_main_pipeline_layout = myvk::PipelineLayout::Create(device, {m_descriptor_set_layout},
	                                                      {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 6}});

	constexpr uint32_t kPathTracerViewerMainVertSpirv[] = {
#include "spirv/path_tracer_viewer_main.vert.u32"
	};
	constexpr uint32_t kPathTracerViewerMainFragSpirv[] = {
#include "spirv/path_tracer_viewer_main.frag.u32"
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module =
	    myvk::ShaderModule::Create(device, kPathTracerViewerMainVertSpirv, sizeof(kPathTracerViewerMainVertSpirv));
	frag_shader_module =
	    myvk::ShaderModule::Create(device, kPathTracerViewerMainFragSpirv, sizeof(kPathTracerViewerMainFragSpirv));

	VkPipelineShaderStageCreateInfo shader_stages[] = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	VkPipelineVertexInputStateCreateInfo vertex_input = {};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)kWidth;
	viewport.height = (float)kHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = {kWidth, kHeight};

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
	color_blend_attachment.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

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

	m_main_graphics_pipeline = myvk::GraphicsPipeline::Create(m_main_pipeline_layout, render_pass, pipeline_info);
}

std::shared_ptr<PathTracerViewer> PathTracerViewer::Create(const std::shared_ptr<PathTracer> &path_tracer,
                                                           const std::shared_ptr<myvk::RenderPass> &render_pass,
                                                           uint32_t subpass) {
	std::shared_ptr<PathTracerViewer> ret = std::make_shared<PathTracerViewer>();
	ret->m_path_tracer_ptr = path_tracer;

	ret->m_image = myvk::Image::CreateTexture2D(
	    render_pass->GetDevicePtr(), {path_tracer->m_width, path_tracer->m_height}, 1, VK_FORMAT_R8G8B8A8_UNORM,
	    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	ret->m_image_view = myvk::ImageView::Create(ret->m_image, VK_IMAGE_VIEW_TYPE_2D);
	ret->m_sampler = myvk::Sampler::CreateClampToBorder(render_pass->GetDevicePtr(), VK_FILTER_NEAREST, {});
	ret->create_render_pass(render_pass->GetDevicePtr());
	ret->m_gen_framebuffer = myvk::Framebuffer::Create(ret->m_gen_render_pass, ret->m_image_view);
	ret->create_gen_graphics_pipeline(render_pass->GetDevicePtr());

	ret->create_descriptors(render_pass->GetDevicePtr());
	ret->create_main_graphics_pipeline(render_pass, subpass);

	return ret;
}

void PathTracerViewer::Reset(const std::shared_ptr<myvk::CommandPool> &command_pool) {
	m_image = myvk::Image::CreateTexture2D(
	    m_gen_render_pass->GetDevicePtr(), {m_path_tracer_ptr->m_width, m_path_tracer_ptr->m_height}, 1,
	    VK_FORMAT_R8G8B8A8_UNORM,
	    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	m_image_view = myvk::ImageView::Create(m_image, VK_IMAGE_VIEW_TYPE_2D);
	m_gen_framebuffer = myvk::Framebuffer::Create(m_gen_render_pass, m_image_view);
	m_descriptor_set->UpdateCombinedImageSampler(m_sampler, m_image_view, 0);

	std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	    {m_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                               VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)});
	command_buffer->CmdClearColorImage(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, {},
	    {m_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, 0,
	                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
	command_buffer->End();

	std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
	command_buffer->Submit(fence);
	fence->Wait();
}

void PathTracerViewer::CmdGenRenderPass(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const {
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_path_tracer_ptr->m_width;
	viewport.height = (float)m_path_tracer_ptr->m_height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = {m_path_tracer_ptr->m_width, m_path_tracer_ptr->m_height};

	uint32_t view_type_u32 = (uint32_t)m_view_type;
	command_buffer->CmdBeginRenderPass(m_gen_render_pass, m_gen_framebuffer, {{{0.0f, 0.0f, 0.0f, 1.0f}}});
	command_buffer->CmdBindPipeline(m_gen_graphics_pipeline);
	command_buffer->CmdBindDescriptorSets({m_path_tracer_ptr->GetTargetDescriptorSet()}, m_gen_graphics_pipeline);

	command_buffer->CmdSetScissor({scissor});
	command_buffer->CmdSetViewport({viewport});

	command_buffer->CmdPushConstants(m_gen_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t),
	                                 &view_type_u32);
	command_buffer->CmdDraw(3, 1, 0, 0);
	command_buffer->CmdEndRenderPass();
}

void PathTracerViewer::CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_main_graphics_pipeline);
	command_buffer->CmdBindDescriptorSets({m_descriptor_set}, m_main_graphics_pipeline);
	// calculate align
	uint32_t pw_sh = m_path_tracer_ptr->m_width * kHeight, ph_sw = m_path_tracer_ptr->m_height * kWidth;
	float texcoords[6] = {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 2.0f};
	if (pw_sh < ph_sw) {
		float x = 0.5f * ph_sw / float(pw_sh) - 0.5f;
		texcoords[0] = texcoords[4] = -x;
		texcoords[2] = 2.0f + x * 3.0f;
	} else if (pw_sh > ph_sw) {
		float x = 0.5f * pw_sh / float(ph_sw) - 0.5f;
		texcoords[1] = texcoords[3] = -x;
		texcoords[5] = 2.0f + x * 3.0f;
	}

	command_buffer->CmdPushConstants(m_main_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(texcoords),
	                                 texcoords);
	command_buffer->CmdDraw(3, 1, 0, 0);
}
