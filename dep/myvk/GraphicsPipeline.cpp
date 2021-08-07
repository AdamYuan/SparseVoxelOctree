#include "GraphicsPipeline.hpp"

namespace myvk {

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::Create(const std::shared_ptr<PipelineLayout> &pipeline_layout,
                                                           const std::shared_ptr<RenderPass> &render_pass,
                                                           const VkGraphicsPipelineCreateInfo &create_info) {
	std::shared_ptr<GraphicsPipeline> ret = std::make_shared<GraphicsPipeline>();
	ret->m_pipeline_layout_ptr = pipeline_layout;
	ret->m_render_pass_ptr = render_pass;

	VkGraphicsPipelineCreateInfo new_info = create_info;
	new_info.renderPass = render_pass->GetHandle();
	new_info.layout = pipeline_layout->GetHandle();

	if (vkCreateGraphicsPipelines(pipeline_layout->GetDevicePtr()->GetHandle(),
	                              pipeline_layout->GetDevicePtr()->GetPipelineCacheHandle(), 1, &new_info, nullptr,
	                              &ret->m_pipeline) != VK_SUCCESS)
		return nullptr;
	return ret;
}
std::shared_ptr<GraphicsPipeline>
GraphicsPipeline::Create(const std::shared_ptr<PipelineLayout> &pipeline_layout,
                         const std::shared_ptr<RenderPass> &render_pass,
                         const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                         const GraphicsPipelineState &pipeline_state, uint32_t subpass) {
	std::shared_ptr<GraphicsPipeline> ret = std::make_shared<GraphicsPipeline>();
	ret->m_pipeline_layout_ptr = pipeline_layout;
	ret->m_render_pass_ptr = render_pass;

	VkGraphicsPipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.renderPass = render_pass->GetHandle();
	create_info.layout = pipeline_layout->GetHandle();
	create_info.stageCount = shader_stages.size();
	create_info.pStages = shader_stages.data();
	pipeline_state.SetGraphicsPipelineCreateInfo(&create_info);
	create_info.subpass = subpass;

	if (vkCreateGraphicsPipelines(pipeline_layout->GetDevicePtr()->GetHandle(),
	                              pipeline_layout->GetDevicePtr()->GetPipelineCacheHandle(), 1, &create_info, nullptr,
	                              &ret->m_pipeline) != VK_SUCCESS)
		return nullptr;
	return ret;
}
void GraphicsPipelineState::RasterizationState::Initialize(VkPolygonMode polygon_mode, VkFrontFace front_face,
                                                           VkCullModeFlags cull_mode) {
	m_create_info.polygonMode = polygon_mode;
	m_create_info.frontFace = front_face;
	m_create_info.cullMode = cull_mode;
}
void GraphicsPipelineState::VertexInputState::Enable(const std::vector<VkVertexInputBindingDescription> &bindings,
                                                     const std::vector<VkVertexInputAttributeDescription> &attributes) {
	m_enable = true;
	m_bindings = bindings;
	m_attributes = attributes;
	m_create_info.vertexBindingDescriptionCount = m_bindings.size();
	m_create_info.pVertexBindingDescriptions = m_bindings.data();
	m_create_info.vertexAttributeDescriptionCount = m_attributes.size();
	m_create_info.pVertexAttributeDescriptions = m_attributes.data();
}
void GraphicsPipelineState::VertexInputState::Enable() {
	m_enable = true;
	m_create_info.vertexBindingDescriptionCount = 0;
	m_create_info.pVertexBindingDescriptions = nullptr;
	m_create_info.vertexAttributeDescriptionCount = 0;
	m_create_info.pVertexAttributeDescriptions = nullptr;
}
void GraphicsPipelineState::InputAssemblyState::Enable(VkPrimitiveTopology topology,
                                                       VkBool32 primitive_restart_enable) {
	m_enable = true;
	m_create_info.topology = topology;
	m_create_info.primitiveRestartEnable = primitive_restart_enable;
}
void GraphicsPipelineState::TessellationState::Enable(uint32_t patch_control_points) {
	m_enable = true;
	m_create_info.patchControlPoints = patch_control_points;
}
void GraphicsPipelineState::ViewportState::Enable(uint32_t viewport_count, uint32_t scissor_count) {
	m_enable = true;
	m_create_info.viewportCount = viewport_count;
	m_create_info.scissorCount = scissor_count;
}
void GraphicsPipelineState::ViewportState::Enable(const std::vector<VkViewport> &viewports,
                                                  const std::vector<VkRect2D> &scissors) {
	m_enable = true;
	m_viewports = viewports;
	m_scissors = scissors;
	m_create_info.viewportCount = m_viewports.size();
	m_create_info.pViewports = m_viewports.data();
	m_create_info.scissorCount = m_scissors.size();
	m_create_info.pScissors = m_scissors.data();
}
void GraphicsPipelineState::MultisampleState::Enable(VkSampleCountFlagBits samples) {
	m_enable = true;
	m_create_info.rasterizationSamples = samples;
}
void GraphicsPipelineState::DepthStencilState::Enable(VkBool32 depth_test_enable, VkBool32 depth_write_enable,
                                                      VkCompareOp depth_compare_op) {
	m_enable = true;
	m_create_info.depthTestEnable = depth_test_enable;
	m_create_info.depthWriteEnable = depth_write_enable;
	m_create_info.depthCompareOp = depth_compare_op;
}
void GraphicsPipelineState::ColorBlendState::Enable(uint32_t attachment_count, VkBool32 blend_enable) {
	m_enable = true;
	m_color_blend_attachments.resize(attachment_count, {});
	if (blend_enable == VK_TRUE)
		for (auto &i : m_color_blend_attachments) {
			i.blendEnable = VK_TRUE;
			i.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			i.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			i.colorBlendOp = VK_BLEND_OP_ADD;
			i.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			i.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			i.alphaBlendOp = VK_BLEND_OP_ADD;
			i.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			                   VK_COLOR_COMPONENT_A_BIT;
		}
	else
		for (auto &i : m_color_blend_attachments) {
			i.blendEnable = VK_FALSE;
			i.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
		}
	m_create_info.attachmentCount = m_color_blend_attachments.size();
	m_create_info.pAttachments = m_color_blend_attachments.data();
}
void GraphicsPipelineState::ColorBlendState::Enable(
    const std::vector<VkPipelineColorBlendAttachmentState> &color_blend_attachments) {
	m_enable = true;
	m_color_blend_attachments = color_blend_attachments;
	m_create_info.attachmentCount = m_color_blend_attachments.size();
	m_create_info.pAttachments = m_color_blend_attachments.data();
}
void GraphicsPipelineState::DynamicState::Enable(const std::vector<VkDynamicState> &dynamic_states) {
	m_enable = true;
	m_dynamic_states = dynamic_states;
	m_create_info.dynamicStateCount = m_dynamic_states.size();
	m_create_info.pDynamicStates = m_dynamic_states.data();
}
void GraphicsPipelineState::SetGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo *info) const {
	info->pRasterizationState = &m_rasterization_state.m_create_info;
	info->pVertexInputState = m_vertex_input_state.m_enable ? &m_vertex_input_state.m_create_info : nullptr;
	info->pInputAssemblyState = m_input_assembly_state.m_enable ? &m_input_assembly_state.m_create_info : nullptr;
	info->pTessellationState = m_tessellation_state.m_enable ? &m_tessellation_state.m_create_info : nullptr;
	info->pViewportState = m_viewport_state.m_enable ? &m_viewport_state.m_create_info : nullptr;
	info->pMultisampleState = m_multisample_state.m_enable ? &m_multisample_state.m_create_info : nullptr;
	info->pDepthStencilState = m_depth_stencil_state.m_enable ? &m_depth_stencil_state.m_create_info : nullptr;
	info->pColorBlendState = m_color_blend_state.m_enable ? &m_color_blend_state.m_create_info : nullptr;
	info->pDynamicState = m_dynamic_state.m_enable ? &m_dynamic_state.m_create_info : nullptr;
}
} // namespace myvk
