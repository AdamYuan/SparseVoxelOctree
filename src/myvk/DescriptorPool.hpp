#ifndef MYVK_DESCRIPTOR_POOL_HPP
#define MYVK_DESCRIPTOR_POOL_HPP

#include "DeviceObjectBase.hpp"

#include <memory>

namespace myvk {
class DescriptorPool : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};

public:
	static std::shared_ptr<DescriptorPool> Create(const std::shared_ptr<Device> &device,
	                                              const VkDescriptorPoolCreateInfo &create_info);

	static std::shared_ptr<DescriptorPool> Create(const std::shared_ptr<Device> &device, uint32_t max_sets,
	                                              const std::vector<VkDescriptorPoolSize> &sizes);

	VkDescriptorPool GetHandle() const { return m_descriptor_pool; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~DescriptorPool();
};
} // namespace myvk

#endif
