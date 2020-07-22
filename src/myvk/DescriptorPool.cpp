#include "DescriptorPool.hpp"

namespace myvk {
	std::shared_ptr<DescriptorPool>
	DescriptorPool::Create(const std::shared_ptr<Device> &device, const VkDescriptorPoolCreateInfo &create_info) {
		std::shared_ptr<DescriptorPool> ret = std::make_shared<DescriptorPool>();
		ret->m_device_ptr = device;
		if (vkCreateDescriptorPool(device->GetHandle(), &create_info, nullptr, &ret->m_descriptor_pool) != VK_SUCCESS)
			return nullptr;
		return ret;
	}

	DescriptorPool::~DescriptorPool() {
		if (m_descriptor_pool)
			vkDestroyDescriptorPool(m_device_ptr->GetHandle(), m_descriptor_pool, nullptr);
	}
}
