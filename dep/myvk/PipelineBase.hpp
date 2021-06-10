#ifndef MYVK_PIPELINE_HPP
#define MYVK_PIPELINE_HPP

#include "DeviceObjectBase.hpp"
#include "PipelineLayout.hpp"
#include <memory>
#include <volk.h>

namespace myvk {
class PipelineBase : public DeviceObjectBase {
protected:
	std::shared_ptr<PipelineLayout> m_pipeline_layout_ptr;
	VkPipeline m_pipeline{VK_NULL_HANDLE};

public:
	VkPipeline GetHandle() const { return m_pipeline; }

	virtual VkPipelineBindPoint GetBindPoint() const = 0;

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_pipeline_layout_ptr->GetDevicePtr(); }
	const std::shared_ptr<PipelineLayout> &GetPipelineLayoutPtr() const { return m_pipeline_layout_ptr; }

	~PipelineBase();
};
} // namespace myvk

#endif
