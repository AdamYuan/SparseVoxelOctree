#include "myvk/FramebufferBase.hpp"

myvk::FramebufferBase::~FramebufferBase() {
	if (m_framebuffer)
		vkDestroyFramebuffer(m_render_pass_ptr->GetDevicePtr()->GetHandle(), m_framebuffer, nullptr);
}
