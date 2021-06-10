#include "RenderPass.hpp"

namespace myvk {
std::shared_ptr<RenderPass> RenderPass::Create(const std::shared_ptr<Device> &device,
                                               const VkRenderPassCreateInfo &create_info) {
	std::shared_ptr<RenderPass> ret = std::make_shared<RenderPass>();
	ret->m_device_ptr = device;

	if (vkCreateRenderPass(device->GetHandle(), &create_info, nullptr, &ret->m_render_pass) != VK_SUCCESS)
		return nullptr;
	return ret;
}

RenderPass::~RenderPass() {
	if (m_render_pass)
		vkDestroyRenderPass(m_device_ptr->GetHandle(), m_render_pass, nullptr);
}
} // namespace myvk
