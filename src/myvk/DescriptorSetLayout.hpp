#ifndef MYVK_DESCRIPTOR_SET_LAYOUT_HPP
#define MYVK_DESCRIPTOR_SET_LAYOUT_HPP

#include "DeviceObjectBase.hpp"

#include <memory>
#include <volk.h>

namespace myvk {
class DescriptorSetLayout : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkDescriptorSetLayout m_descriptor_set_layout{nullptr};

public:
	static std::shared_ptr<DescriptorSetLayout> Create(const std::shared_ptr<Device> &device,
	                                                   const std::vector<VkDescriptorSetLayoutBinding> &bindings);

	VkDescriptorSetLayout GetHandle() const { return m_descriptor_set_layout; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~DescriptorSetLayout();
};
} // namespace myvk

#endif
