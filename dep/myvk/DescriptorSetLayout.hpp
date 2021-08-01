#ifndef MYVK_DESCRIPTOR_SET_LAYOUT_HPP
#define MYVK_DESCRIPTOR_SET_LAYOUT_HPP

#include "DeviceObjectBase.hpp"

#include <memory>
#include <volk.h>

namespace myvk {

class DescriptorBindingFlagGroup;
class DescriptorSetLayout : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};

public:
	static std::shared_ptr<DescriptorSetLayout> Create(const std::shared_ptr<Device> &device,
	                                                   const std::vector<VkDescriptorSetLayoutBinding> &bindings);
	static std::shared_ptr<DescriptorSetLayout> Create(const std::shared_ptr<Device> &device,
	                                                   const DescriptorBindingFlagGroup &binding_flags);

	VkDescriptorSetLayout GetHandle() const { return m_descriptor_set_layout; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~DescriptorSetLayout();
};

class DescriptorBindingFlagGroup {
private:
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	std::vector<VkDescriptorBindingFlags> m_flags;

public:
	DescriptorBindingFlagGroup() = default;

	DescriptorBindingFlagGroup(
	    const std::initializer_list<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags);

	explicit DescriptorBindingFlagGroup(
	    const std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags);

	void
	Initialize(const std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> &binding_flags);

	uint32_t GetCount() const { return m_bindings.size(); }

	const VkDescriptorSetLayoutBinding *GetBindingsPtr() const { return m_bindings.data(); }

	const VkDescriptorBindingFlags *GetFlagsPtr() const { return m_flags.data(); }
};
} // namespace myvk

#endif
