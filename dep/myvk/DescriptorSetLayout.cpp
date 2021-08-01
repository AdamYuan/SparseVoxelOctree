#include "DescriptorSetLayout.hpp"
#include <cstdio>

namespace myvk {
std::shared_ptr<DescriptorSetLayout>
DescriptorSetLayout::Create(const std::shared_ptr<Device> &device,
                            const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
	std::shared_ptr<DescriptorSetLayout> ret = std::make_shared<DescriptorSetLayout>();
	ret->m_device_ptr = device;

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = bindings.size();
	info.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &ret->m_descriptor_set_layout) != VK_SUCCESS)
		return nullptr;

	return ret;
}

std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::Create(const std::shared_ptr<Device> &device,
                                                                 const DescriptorBindingFlagGroup &binding_flags) {
	std::shared_ptr<DescriptorSetLayout> ret = std::make_shared<DescriptorSetLayout>();
	ret->m_device_ptr = device;

	VkDescriptorSetLayoutBindingFlagsCreateInfo flags_info = {};
	flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	flags_info.bindingCount = binding_flags.GetCount();
	flags_info.pBindingFlags = binding_flags.GetFlagsPtr();

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = binding_flags.GetCount();
	info.pBindings = binding_flags.GetBindingsPtr();
	info.pNext = &flags_info;

	if (vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &ret->m_descriptor_set_layout) != VK_SUCCESS)
		return nullptr;

	return ret;
}

DescriptorSetLayout::~DescriptorSetLayout() {
	if (m_descriptor_set_layout)
		vkDestroyDescriptorSetLayout(m_device_ptr->GetHandle(), m_descriptor_set_layout, nullptr);
}

DescriptorBindingFlagGroup::DescriptorBindingFlagGroup(
    const std::initializer_list<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags) {
	Initialize(binding_flags);
}
DescriptorBindingFlagGroup::DescriptorBindingFlagGroup(
    const std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags) {
	Initialize(binding_flags);
}
void DescriptorBindingFlagGroup::Initialize(
    const std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags) {
	m_bindings.clear();
	m_flags.clear();
	for (const auto &i : binding_flags) {
		m_bindings.push_back(i.first);
		m_flags.push_back(i.second);
	}
}
} // namespace myvk
