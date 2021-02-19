#ifndef MYVK_PIPELINE_LAYOUT_HPP
#define MYVK_PIPELINE_LAYOUT_HPP

#include "DescriptorSetLayout.hpp"
#include "DeviceObjectBase.hpp"

#include <volk.h>

#include <memory>
#include <vector>

namespace myvk {
class PipelineLayout : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	std::vector<std::shared_ptr<DescriptorSetLayout>> m_descriptor_layout_ptrs;

	VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

public:
	static std::shared_ptr<PipelineLayout>
	Create(const std::shared_ptr<Device> &device,
	       const std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptor_layouts,
	       const std::vector<VkPushConstantRange> &push_constant_ranges);

	VkPipelineLayout GetHandle() const { return m_pipeline_layout; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	const std::vector<std::shared_ptr<DescriptorSetLayout>> &GetDescriptorSetPtrs() const {
		return m_descriptor_layout_ptrs;
	}

	~PipelineLayout();
};
} // namespace myvk

#endif
