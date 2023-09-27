#ifndef MYVK_FRAMEBUFFER_BASE_HPP
#define MYVK_FRAMEBUFFER_BASE_HPP

#include "DeviceObjectBase.hpp"
#include "RenderPass.hpp"
#include "volk.h"

namespace myvk {
class FramebufferBase : public DeviceObjectBase {
protected:
	Ptr<RenderPass> m_render_pass_ptr;

	VkFramebuffer m_framebuffer{VK_NULL_HANDLE};

	VkExtent2D m_extent{};
	uint32_t m_layers{};

public:
	VkFramebuffer GetHandle() const { return m_framebuffer; }

	const Ptr<Device> &GetDevicePtr() const override { return m_render_pass_ptr->GetDevicePtr(); }
	const Ptr<RenderPass> &GetRenderPassPtr() const { return m_render_pass_ptr; }

	const VkExtent2D &GetExtent() const { return m_extent; }
	uint32_t GetLayers() const { return m_layers; }

	~FramebufferBase() override;
};
} // namespace myvk

#endif
