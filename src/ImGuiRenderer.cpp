#include "ImGuiRenderer.hpp"
#include "myvk/Buffer.hpp"
#include "myvk/CommandBuffer.hpp"
#include "myvk/ShaderModule.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

void ImGuiRenderer::Initialize(const std::shared_ptr<myvk::CommandPool> &command_pool,
                               const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass,
                               uint32_t frame_count) {
	create_font_texture(command_pool);
	create_descriptor(render_pass->GetDevicePtr());
	create_pipeline(render_pass, subpass);
	m_vertex_buffers.resize(frame_count);
	m_index_buffers.resize(frame_count);
}

void ImGuiRenderer::create_font_texture(const std::shared_ptr<myvk::CommandPool> &command_pool) {
	unsigned char *data;
	int width, height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&data, &width, &height);
	uint32_t data_size = width * height * 4;

	m_font_texture = myvk::Image::CreateTexture2D(command_pool->GetDevicePtr(), {(uint32_t)width, (uint32_t)height}, 1,
	                                              VK_FORMAT_R8G8B8A8_UNORM,
	                                              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	ImGui::GetIO().Fonts->SetTexID((ImTextureID)(intptr_t)m_font_texture->GetHandle());

	{
		std::shared_ptr<myvk::Fence> fence = myvk::Fence::Create(command_pool->GetDevicePtr());
		std::shared_ptr<myvk::Buffer> staging_buffer =
		    myvk::Buffer::CreateStaging(command_pool->GetDevicePtr(), data_size);
		staging_buffer->UpdateData(data, data + data_size);

		std::shared_ptr<myvk::CommandBuffer> command_buffer = myvk::CommandBuffer::Create(command_pool);
		command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
		    m_font_texture->GetDstMemoryBarriers({region}, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
		command_buffer->CmdCopy(staging_buffer, m_font_texture, {region});
		command_buffer->CmdPipelineBarrier(
		    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {},
		    m_font_texture->GetDstMemoryBarriers({region}, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

		command_buffer->End();

		command_buffer->Submit(fence);
		fence->Wait();
	}

	m_font_texture_view = myvk::ImageView::Create(m_font_texture, VK_IMAGE_VIEW_TYPE_2D, m_font_texture->GetFormat(),
	                                              VK_IMAGE_ASPECT_COLOR_BIT);
	m_font_texture_sampler =
	    myvk::Sampler::Create(command_pool->GetDevicePtr(), VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

void ImGuiRenderer::create_descriptor(const std::shared_ptr<myvk::Device> &device) {
	{
		VkDescriptorSetLayoutBinding layout_binding = {};
		layout_binding.binding = 0;
		layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_binding.descriptorCount = 1;
		layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkSampler immutable_samplers[] = {m_font_texture_sampler->GetHandle()};
		layout_binding.pImmutableSamplers = immutable_samplers;

		m_descriptor_set_layout = myvk::DescriptorSetLayout::Create(device, {layout_binding});
	}
	m_descriptor_pool = myvk::DescriptorPool::Create(device, 1, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}});
	m_descriptor_set = myvk::DescriptorSet::Create(m_descriptor_pool, m_descriptor_set_layout);
	m_descriptor_set->UpdateCombinedImageSampler(m_font_texture_sampler, m_font_texture_view, 0);
}

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------
constexpr uint32_t kShaderVertSpv[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
    0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015, 0x0000001b, 0x0000001c, 0x00030003,
    0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00030005, 0x00000009, 0x00000000,
    0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x00000009, 0x00000001, 0x00005655,
    0x00030005, 0x0000000b, 0x0074754f, 0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015,
    0x00565561, 0x00060005, 0x00000019, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000019,
    0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
    0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368, 0x6e617473, 0x00000074, 0x00050006,
    0x0000001e, 0x00000000, 0x61635375, 0x0000656c, 0x00060006, 0x0000001e, 0x00000001, 0x61725475, 0x616c736e,
    0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00040047,
    0x0000000f, 0x0000001e, 0x00000002, 0x00040047, 0x00000015, 0x0000001e, 0x00000001, 0x00050048, 0x00000019,
    0x00000000, 0x0000000b, 0x00000000, 0x00030047, 0x00000019, 0x00000002, 0x00040047, 0x0000001c, 0x0000001e,
    0x00000000, 0x00050048, 0x0000001e, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001,
    0x00000023, 0x00000008, 0x00030047, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
    0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007, 0x00000008, 0x00040020, 0x0000000a,
    0x00000003, 0x00000009, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000003, 0x00040015, 0x0000000c, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020, 0x0000000e, 0x00000001, 0x00000007,
    0x0004003b, 0x0000000e, 0x0000000f, 0x00000001, 0x00040020, 0x00000011, 0x00000003, 0x00000007, 0x0004002b,
    0x0000000c, 0x00000013, 0x00000001, 0x00040020, 0x00000014, 0x00000001, 0x00000008, 0x0004003b, 0x00000014,
    0x00000015, 0x00000001, 0x00040020, 0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007,
    0x00040020, 0x0000001a, 0x00000003, 0x00000019, 0x0004003b, 0x0000001a, 0x0000001b, 0x00000003, 0x0004003b,
    0x00000014, 0x0000001c, 0x00000001, 0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
    0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009, 0x00040020, 0x00000021, 0x00000009,
    0x00000008, 0x0004002b, 0x00000006, 0x00000028, 0x00000000, 0x0004002b, 0x00000006, 0x00000029, 0x3f800000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007,
    0x00000010, 0x0000000f, 0x00050041, 0x00000011, 0x00000012, 0x0000000b, 0x0000000d, 0x0003003e, 0x00000012,
    0x00000010, 0x0004003d, 0x00000008, 0x00000016, 0x00000015, 0x00050041, 0x00000017, 0x00000018, 0x0000000b,
    0x00000013, 0x0003003e, 0x00000018, 0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041,
    0x00000021, 0x00000022, 0x00000020, 0x0000000d, 0x0004003d, 0x00000008, 0x00000023, 0x00000022, 0x00050085,
    0x00000008, 0x00000024, 0x0000001d, 0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
    0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008, 0x00000027, 0x00000024, 0x00000026,
    0x00050051, 0x00000006, 0x0000002a, 0x00000027, 0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027,
    0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b, 0x00000028, 0x00000029, 0x00050041,
    0x00000011, 0x0000002d, 0x0000001b, 0x0000000d, 0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038};

constexpr uint32_t kShaderFragSpv[] = {
    0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001,
    0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010, 0x00000004, 0x00000007, 0x00030003,
    0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x6c6f4366,
    0x0000726f, 0x00030005, 0x0000000b, 0x00000000, 0x00050006, 0x0000000b, 0x00000000, 0x6f6c6f43, 0x00000072,
    0x00040006, 0x0000000b, 0x00000001, 0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016,
    0x78655473, 0x65727574, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d,
    0x0000001e, 0x00000000, 0x00040047, 0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
    0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020,
    0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
    0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000002, 0x0004001e, 0x0000000b,
    0x00000007, 0x0000000a, 0x00040020, 0x0000000c, 0x00000001, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000001, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
    0x00040020, 0x00000010, 0x00000001, 0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x00000014, 0x00000013, 0x00040020, 0x00000015,
    0x00000000, 0x00000014, 0x0004003b, 0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
    0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x00050041, 0x00000010, 0x00000011, 0x0000000d, 0x0000000f, 0x0004003d,
    0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017, 0x00000016, 0x00050041, 0x00000019,
    0x0000001a, 0x0000000d, 0x00000018, 0x0004003d, 0x0000000a, 0x0000001b, 0x0000001a, 0x00050057, 0x00000007,
    0x0000001c, 0x00000017, 0x0000001b, 0x00050085, 0x00000007, 0x0000001d, 0x00000012, 0x0000001c, 0x0003003e,
    0x00000009, 0x0000001d, 0x000100fd, 0x00010038};

void ImGuiRenderer::create_pipeline(const std::shared_ptr<myvk::RenderPass> &render_pass, uint32_t subpass) {
	const std::shared_ptr<myvk::Device> &device = render_pass->GetDevicePtr();
	{
		VkPushConstantRange push_constant = {};
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		push_constant.offset = sizeof(float) * 0;
		push_constant.size = sizeof(float) * 4;

		m_pipeline_layout = myvk::PipelineLayout::Create(device, {m_descriptor_set_layout}, {push_constant});
	}
	{
		std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
		vert_shader_module = myvk::ShaderModule::Create(device, kShaderVertSpv, sizeof(kShaderVertSpv));
		frag_shader_module = myvk::ShaderModule::Create(device, kShaderFragSpv, sizeof(kShaderFragSpv));

		VkPipelineShaderStageCreateInfo shader_stages[] = {
		    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

		VkVertexInputBindingDescription binding_desc[1] = {};
		binding_desc[0].stride = sizeof(ImDrawVert);
		binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attribute_desc[3] = {};
		attribute_desc[0].location = 0;
		attribute_desc[0].binding = binding_desc[0].binding;
		attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
		attribute_desc[1].location = 1;
		attribute_desc[1].binding = binding_desc[0].binding;
		attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
		attribute_desc[2].location = 2;
		attribute_desc[2].binding = binding_desc[0].binding;
		attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = binding_desc;
		vertex_input_info.vertexAttributeDescriptionCount = 3;
		vertex_input_info.pVertexAttributeDescriptions = attribute_desc;

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
		dynamic_state.pDynamicStates = dynamic_states;

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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = nullptr;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = &dynamic_state;
		pipeline_info.subpass = subpass;

		m_pipeline = myvk::GraphicsPipeline::Create(m_pipeline_layout, render_pass, pipeline_info);
	}
}

void ImGuiRenderer::CmdDrawPipeline(const std::shared_ptr<myvk::CommandBuffer> &command_buffer,
                                    uint32_t current_frame) {
	const std::shared_ptr<myvk::Device> &device = command_buffer->GetDevicePtr();

	ImDrawData *draw_data = ImGui::GetDrawData();

	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0 || draw_data->TotalVtxCount == 0)
		return;

	auto &vertex_buffer = m_vertex_buffers[current_frame];
	auto &index_buffer = m_index_buffers[current_frame];
	// Fill vertex buffer and index buffer
	{
		VkDeviceSize vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (!vertex_buffer || vertex_buffer->GetSize() < vertex_size || vertex_buffer->GetSize() > vertex_size * 4)
			vertex_buffer = myvk::Buffer::Create(device, vertex_size * 2, VMA_MEMORY_USAGE_CPU_TO_GPU,
			                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		if (!index_buffer || index_buffer->GetSize() < index_size || index_buffer->GetSize() > index_size * 4)
			index_buffer = myvk::Buffer::Create(device, index_size * 2, VMA_MEMORY_USAGE_CPU_TO_GPU,
			                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		auto *vertex_map = (ImDrawVert *)vertex_buffer->Map();
		auto *index_map = (ImDrawIdx *)index_buffer->Map();

		for (int i = 0; i < draw_data->CmdListsCount; ++i) {
			const ImDrawList *cmd_list = draw_data->CmdLists[i];
			std::copy(cmd_list->VtxBuffer.begin(), cmd_list->VtxBuffer.end(), vertex_map);
			std::copy(cmd_list->IdxBuffer.begin(), cmd_list->IdxBuffer.end(), index_map);
			vertex_map += cmd_list->VtxBuffer.Size;
			index_map += cmd_list->IdxBuffer.Size;
		}

		vertex_buffer->Unmap();
		index_buffer->Unmap();
	}

	setup_render_state(command_buffer, fb_width, fb_height, current_frame);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback) {
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer
				// to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					setup_render_state(command_buffer, fb_width, fb_height, current_frame);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			} else {
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < (float)fb_width && clip_rect.y < (float)fb_height && clip_rect.z >= 0.0f &&
				    clip_rect.w >= 0.0f) {
					// Negative offsets are illegal for vkCmdSetScissor
					if (clip_rect.x < 0.0f)
						clip_rect.x = 0.0f;
					if (clip_rect.y < 0.0f)
						clip_rect.y = 0.0f;

					// Apply scissor/clipping rectangle
					VkRect2D scissor;
					scissor.offset.x = (int32_t)clip_rect.x;
					scissor.offset.y = (int32_t)clip_rect.y;
					scissor.extent.width = (uint32_t)(clip_rect.z - clip_rect.x);
					scissor.extent.height = (uint32_t)(clip_rect.w - clip_rect.y);
					command_buffer->CmdSetScissor({scissor});
					command_buffer->CmdDrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset,
					                               pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

void ImGuiRenderer::setup_render_state(const std::shared_ptr<myvk::CommandBuffer> &command_buffer, int fb_width,
                                       int fb_height, uint32_t current_frame) const {
	ImDrawData *draw_data = ImGui::GetDrawData();

	{
		command_buffer->CmdBindPipeline(m_pipeline);
		command_buffer->CmdBindDescriptorSets({m_descriptor_set}, m_pipeline, {});
		command_buffer->CmdBindVertexBuffer(m_vertex_buffers[current_frame], 0);
		command_buffer->CmdBindIndexBuffer(m_index_buffers[current_frame], 0,
		                                   sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}

	// Setup viewport:
	{
		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)fb_width;
		viewport.height = (float)fb_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		command_buffer->CmdSetViewport({viewport});
	}

	// Setup scale and translation:
	// Our visible imgui space lies from draw_data->DisplayPps (top left) to
	// draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	{
		float scale[] = {2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y};
		float translate[] = {-1.0f - draw_data->DisplayPos.x * scale[0], -1.0f - draw_data->DisplayPos.y * scale[1]};
		command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0,
		                                 sizeof(scale), scale);
		command_buffer->CmdPushConstants(m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2,
		                                 sizeof(translate), translate);
	}
}
