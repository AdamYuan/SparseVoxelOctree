#include "OctreeTracer.hpp"
#include "Config.hpp"
#include "QuadSpirv.hpp"
#include "myvk/ShaderModule.hpp"

constexpr uint32_t get_beam_size(uint32_t x) { return (x + (kBeamSize - 1)) / kBeamSize + 1; }

void OctreeTracer::create_descriptor_pool(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	m_descriptor_pool =
	    myvk::DescriptorPool::Create(device, frame_count, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, frame_count}});
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
	m_main_pipeline_layout = myvk::PipelineLayout::Create(
	    device,
	    {m_octree_ptr->GetDescriptorSetLayout(), m_camera_ptr->GetDescriptorSetLayout(),
	     m_lighting_ptr->GetEnvironmentMapPtr()->GetDescriptorSetLayout(), m_descriptor_set_layout},
	    {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 6 * sizeof(uint32_t) + 4 * sizeof(float)}});
	m_beam_pipeline_layout = myvk::PipelineLayout::Create(
	    device, {m_octree_ptr->GetDescriptorSetLayout(), m_camera_ptr->GetDescriptorSetLayout()},
	    {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, 3 * sizeof(uint32_t) + sizeof(float)}});
}

void OctreeTracer::create_frame_resources(const std::shared_ptr<myvk::Device> &device, uint32_t frame_count) {
	m_frame_resources.resize(frame_count);

	for (auto &i : m_frame_resources) {
		i.m_beam_image = myvk::Image::CreateTexture2D(device, {get_beam_size(m_width), get_beam_size(m_height)}, 1,
		                                              VK_FORMAT_R32_SFLOAT,
		                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		i.m_beam_image_view = myvk::ImageView::Create(i.m_beam_image, VK_IMAGE_VIEW_TYPE_2D);
		i.m_beam_sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		i.m_beam_framebuffer = myvk::Framebuffer::Create(m_beam_render_pass, i.m_beam_image_view);
		i.m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
		i.m_descriptor_set->UpdateCombinedImageSampler(i.m_beam_sampler, i.m_beam_image_view, 0);
	}
}

void OctreeTracer::create_main_graphics_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass,
                                                 uint32_t subpass) {
	constexpr uint32_t kOctreeTracerFragSpv[] = {
#include "spirv/octree_tracer.frag.u32"
	};
	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kQuadVertSpv, sizeof(kQuadVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kOctreeTracerFragSpv, sizeof(kOctreeTracerFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable();
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_viewport_state.Enable(1, 1);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_FRONT_BIT);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_color_blend_state.Enable(1, VK_FALSE);
	pipeline_state.m_dynamic_state.Enable({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});

	m_main_graphics_pipeline =
	    myvk::GraphicsPipeline::Create(m_main_pipeline_layout, render_pass, shader_stages, pipeline_state, subpass);
}

void OctreeTracer::create_beam_render_pass(const std::shared_ptr<myvk::Device> &device) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = VK_FORMAT_R32_SFLOAT;
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

	m_beam_render_pass = myvk::RenderPass::Create(device, render_pass_info);
}

void OctreeTracer::create_beam_graphics_pipeline(const std::shared_ptr<myvk::Device> &device) {
	constexpr uint32_t kOctreeTracerBeamFragSpv[] = {
#include "spirv/octree_tracer_beam.frag.u32"
	};
	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kQuadVertSpv, sizeof(kQuadVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kOctreeTracerBeamFragSpv, sizeof(kOctreeTracerBeamFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable();
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_viewport_state.Enable(1, 1);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_FRONT_BIT);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_color_blend_state.Enable(1, VK_FALSE);
	pipeline_state.m_dynamic_state.Enable({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});

	m_beam_graphics_pipeline =
	    myvk::GraphicsPipeline::Create(m_beam_pipeline_layout, m_beam_render_pass, shader_stages, pipeline_state, 0);
}

std::shared_ptr<OctreeTracer> OctreeTracer::Create(const std::shared_ptr<Octree> &octree,
                                                   const std::shared_ptr<Camera> &camera,
                                                   const std::shared_ptr<Lighting> &lighting,
                                                   const std::shared_ptr<myvk::RenderPass> &render_pass,
                                                   uint32_t subpass, uint32_t frame_count) {
	std::shared_ptr<OctreeTracer> ret = std::make_shared<OctreeTracer>();
	ret->m_octree_ptr = octree;
	ret->m_camera_ptr = camera;
	ret->m_lighting_ptr = lighting;

	std::shared_ptr<myvk::Device> device = render_pass->GetDevicePtr();
	ret->create_descriptor_pool(device, frame_count);
	ret->create_layouts(device);
	ret->create_beam_render_pass(device);
	ret->create_frame_resources(device, frame_count);
	ret->create_main_graphics_pipeline(render_pass, subpass);
	ret->create_beam_graphics_pipeline(device);

	return ret;
}

void OctreeTracer::Resize(uint32_t width, uint32_t height) {
	m_width = width;
	m_height = height;

	uint32_t beam_width = get_beam_size(m_width), beam_height = get_beam_size(m_height);
	if (m_frame_resources[0].m_beam_image->GetExtent().width != beam_width ||
	    m_frame_resources[0].m_beam_image->GetExtent().height != beam_height) {
		for (auto &i : m_frame_resources) {
			i.m_beam_image = myvk::Image::CreateTexture2D(
			    i.m_beam_image->GetDevicePtr(), {beam_width, beam_height}, 1, VK_FORMAT_R32_SFLOAT,
			    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			i.m_beam_image_view = myvk::ImageView::Create(i.m_beam_image, VK_IMAGE_VIEW_TYPE_2D);
			i.m_beam_framebuffer = myvk::Framebuffer::Create(m_beam_render_pass, i.m_beam_image_view);
			i.m_descriptor_set->UpdateCombinedImageSampler(i.m_beam_sampler, i.m_beam_image_view, 0);
		}
	}
}

void OctreeTracer::CmdBeamRenderPass(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
                                     uint32_t current_frame) const {
	if (!m_beam_enable)
		return;
	const auto &cur = m_frame_resources[current_frame];
	command_buffer->CmdBeginRenderPass(m_beam_render_pass, cur.m_beam_framebuffer, {{{0.0f, 0.0f, 0.0f, 1.0f}}});
	command_buffer->CmdBindPipeline(m_beam_graphics_pipeline);
	command_buffer->CmdBindDescriptorSets(
	    {m_octree_ptr->GetDescriptorSet(), m_camera_ptr->GetFrameDescriptorSet(current_frame)},
	    m_beam_graphics_pipeline);

	uint32_t beam_width = get_beam_size(m_width), beam_height = get_beam_size(m_height);
	VkRect2D scissor = {};
	scissor.extent = {beam_width, beam_height};
	command_buffer->CmdSetScissor({scissor});
	VkViewport viewport = {};
	viewport.width = beam_width;
	viewport.height = beam_height;
	command_buffer->CmdSetViewport({viewport});

	uint32_t uint_push_constants[] = {m_width, m_height, kBeamSize};
	command_buffer->CmdPushConstants(m_beam_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
	                                 sizeof(uint_push_constants), uint_push_constants);
	float origin_size = exp2f(1.0f - m_octree_ptr->GetLevel());
	command_buffer->CmdPushConstants(m_beam_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint_push_constants),
	                                 sizeof(float), &origin_size);
	command_buffer->CmdDraw(3, 1, 0, 0);
	command_buffer->CmdEndRenderPass();
}

void OctreeTracer::CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
                                   uint32_t current_frame) const {
	const auto &cur = m_frame_resources[current_frame];
	command_buffer->CmdBindPipeline(m_main_graphics_pipeline);
	command_buffer->CmdBindDescriptorSets(
	    {m_octree_ptr->GetDescriptorSet(), m_camera_ptr->GetFrameDescriptorSet(current_frame),
	     m_lighting_ptr->GetEnvironmentMapPtr()->GetDescriptorSet(), cur.m_descriptor_set},
	    m_main_graphics_pipeline);

	VkRect2D scissor = {};
	scissor.extent = {m_width, m_height};
	command_buffer->CmdSetScissor({scissor});
	VkViewport viewport = {};
	viewport.width = m_width;
	viewport.height = m_height;
	command_buffer->CmdSetViewport({viewport});

	uint32_t uint_push_constants[] = {
	    m_width,       m_height, (uint32_t)m_view_type, (uint32_t)m_lighting_ptr->GetFinalLightType(),
	    m_beam_enable, kBeamSize};
	glm::vec3 sun_color = m_lighting_ptr->m_sun_radiance;
	float max_radiance = glm::max(glm::max(sun_color.r, sun_color.g), sun_color.b);
	if (max_radiance != 0.0f)
		sun_color /= max_radiance;

	float float_push_constants[] = {sun_color.x, sun_color.y, sun_color.z,
	                                m_lighting_ptr->GetEnvironmentMapPtr()->m_rotation};

	command_buffer->CmdPushConstants(m_main_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
	                                 sizeof(uint_push_constants), uint_push_constants);
	command_buffer->CmdPushConstants(m_main_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint_push_constants),
	                                 sizeof(float_push_constants), float_push_constants);
	command_buffer->CmdDraw(3, 1, 0, 0);
}
