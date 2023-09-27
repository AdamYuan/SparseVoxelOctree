#ifndef MYVK_PIPELINE_LAYOUT_HPP
#define MYVK_PIPELINE_LAYOUT_HPP

#include "DescriptorSetLayout.hpp"
#include "DeviceObjectBase.hpp"

#include "volk.h"

#include <memory>
#include <vector>

namespace myvk {
class PipelineLayout : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	std::vector<Ptr<DescriptorSetLayout>> m_descriptor_layout_ptrs;

	VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

public:
	static Ptr<PipelineLayout> Create(const Ptr<Device> &device,
	                                  const std::vector<Ptr<DescriptorSetLayout>> &descriptor_layouts,
	                                  const std::vector<VkPushConstantRange> &push_constant_ranges);

	VkPipelineLayout GetHandle() const { return m_pipeline_layout; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	const std::vector<Ptr<DescriptorSetLayout>> &GetDescriptorSetPtrs() const { return m_descriptor_layout_ptrs; }

	~PipelineLayout() override;
};
} // namespace myvk

#endif
