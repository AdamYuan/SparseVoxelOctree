#include "DescriptorSetLayout.hpp"
#include <cstdio>

namespace myvk {
	std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::Create(const std::shared_ptr<Device> &device,
																	 const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
		std::shared_ptr<DescriptorSetLayout> ret = std::make_shared<DescriptorSetLayout>();
		ret->m_device_ptr = device;

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = bindings.size();
		info.pBindings = bindings.data();

		if(vkCreateDescriptorSetLayout(device->GetHandle(), &info, nullptr, &ret->m_descriptor_set_layout) != VK_SUCCESS)
			return nullptr;

		return ret;
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		if(m_descriptor_set_layout)
			vkDestroyDescriptorSetLayout(m_device_ptr->GetHandle(), m_descriptor_set_layout, nullptr);
	}
}
