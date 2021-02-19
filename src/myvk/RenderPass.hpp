#ifndef MYVK_RENDER_PASS_HPP
#define MYVK_RENDER_PASS_HPP

#include "DeviceObjectBase.hpp"
#include <memory>
#include <volk.h>

namespace myvk {
class RenderPass : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkRenderPass m_render_pass{VK_NULL_HANDLE};

public:
	static std::shared_ptr<RenderPass> Create(const std::shared_ptr<Device> &device,
	                                          const VkRenderPassCreateInfo &create_info);

	VkRenderPass GetHandle() const { return m_render_pass; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~RenderPass();
};
} // namespace myvk

#endif
