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
} // namespace myvk
