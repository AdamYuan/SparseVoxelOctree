#include "DescriptorPool.hpp"

namespace myvk {
std::shared_ptr<DescriptorPool> DescriptorPool::Create(const std::shared_ptr<Device> &device,
                                                       const VkDescriptorPoolCreateInfo &create_info) {
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

std::shared_ptr<DescriptorPool> DescriptorPool::Create(const std::shared_ptr<Device> &device, uint32_t max_sets,
                                                       const std::vector<VkDescriptorPoolSize> &sizes) {
	VkDescriptorPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	create_info.maxSets = max_sets;
	create_info.poolSizeCount = sizes.size();
	create_info.pPoolSizes = sizes.data();

	return Create(device, create_info);
}
} // namespace myvk
