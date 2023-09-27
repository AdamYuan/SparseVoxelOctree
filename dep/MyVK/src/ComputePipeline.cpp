#include "myvk/ComputePipeline.hpp"

namespace myvk {

Ptr<ComputePipeline> ComputePipeline::Create(const Ptr<PipelineLayout> &pipeline_layout,
                                             const VkComputePipelineCreateInfo &create_info) {
	auto ret = std::make_shared<ComputePipeline>();
	ret->m_pipeline_layout_ptr = pipeline_layout;

	VkComputePipelineCreateInfo new_info = create_info;
	new_info.layout = pipeline_layout->GetHandle();

	if (vkCreateComputePipelines(pipeline_layout->GetDevicePtr()->GetHandle(),
	                             pipeline_layout->GetDevicePtr()->GetPipelineCacheHandle(), 1, &new_info, nullptr,
	                             &ret->m_pipeline) != VK_SUCCESS)
		return nullptr;
	return ret;
}

Ptr<ComputePipeline> ComputePipeline::Create(const Ptr<PipelineLayout> &pipeline_layout,
                                             const Ptr<ShaderModule> &shader_module,
                                             VkSpecializationInfo *p_specialization_info) {
	std::shared_ptr<ComputePipeline> ret = std::make_shared<ComputePipeline>();
	ret->m_pipeline_layout_ptr = pipeline_layout;

	VkComputePipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	create_info.layout = pipeline_layout->GetHandle();
	create_info.stage =
	    shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, p_specialization_info);

	if (vkCreateComputePipelines(pipeline_layout->GetDevicePtr()->GetHandle(),
	                             pipeline_layout->GetDevicePtr()->GetPipelineCacheHandle(), 1, &create_info, nullptr,
	                             &ret->m_pipeline) != VK_SUCCESS)
		return nullptr;
	return ret;
}
} // namespace myvk
