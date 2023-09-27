#ifndef MYVK_DESCRIPTOR_POOL_HPP
#define MYVK_DESCRIPTOR_POOL_HPP

#include "DeviceObjectBase.hpp"

#include <memory>

namespace myvk {
class DescriptorPool : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};

public:
	static Ptr<DescriptorPool> Create(const Ptr<Device> &device, const VkDescriptorPoolCreateInfo &create_info);

	static Ptr<DescriptorPool> Create(const Ptr<Device> &device, uint32_t max_sets,
	                                  const std::vector<VkDescriptorPoolSize> &sizes);

	VkDescriptorPool GetHandle() const { return m_descriptor_pool; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~DescriptorPool() override;
};
} // namespace myvk

#endif
