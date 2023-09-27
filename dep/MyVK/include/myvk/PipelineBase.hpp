#ifndef MYVK_PIPELINE_HPP
#define MYVK_PIPELINE_HPP

#include "DeviceObjectBase.hpp"
#include "PipelineLayout.hpp"
#include "volk.h"
#include <memory>

namespace myvk {
class PipelineBase : public DeviceObjectBase {
protected:
	Ptr<PipelineLayout> m_pipeline_layout_ptr;
	VkPipeline m_pipeline{VK_NULL_HANDLE};

public:
	VkPipeline GetHandle() const { return m_pipeline; }

	virtual VkPipelineBindPoint GetBindPoint() const = 0;

	const Ptr<Device> &GetDevicePtr() const override { return m_pipeline_layout_ptr->GetDevicePtr(); }
	const Ptr<PipelineLayout> &GetPipelineLayoutPtr() const { return m_pipeline_layout_ptr; }

	~PipelineBase() override;
};
} // namespace myvk

#endif
