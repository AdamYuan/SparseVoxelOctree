#include "PipelineLayout.hpp"

namespace myvk {

std::shared_ptr<PipelineLayout>
PipelineLayout::Create(const std::shared_ptr<Device> &device,
                       const std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptor_layouts,
                       const std::vector<VkPushConstantRange> &push_constant_ranges) {
	std::shared_ptr<PipelineLayout> ret = std::make_shared<PipelineLayout>();
	ret->m_device_ptr = device;

	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	std::vector<VkDescriptorSetLayout> descriptor_layout_handles(descriptor_layouts.size());
	descriptor_layout_handles.resize(descriptor_layouts.size());
	for (size_t i = 0; i < descriptor_layouts.size(); ++i)
		descriptor_layout_handles[i] = descriptor_layouts[i]->GetHandle();
	info.setLayoutCount = descriptor_layouts.size();
	info.pSetLayouts = descriptor_layout_handles.data();

	info.pushConstantRangeCount = push_constant_ranges.size();
	info.pPushConstantRanges = push_constant_ranges.data();

	if (vkCreatePipelineLayout(device->GetHandle(), &info, nullptr, &ret->m_pipeline_layout) != VK_SUCCESS)
		return nullptr;

	return ret;
}

PipelineLayout::~PipelineLayout() {
	if (m_pipeline_layout)
		vkDestroyPipelineLayout(m_device_ptr->GetHandle(), m_pipeline_layout, nullptr);
}
} // namespace myvk
