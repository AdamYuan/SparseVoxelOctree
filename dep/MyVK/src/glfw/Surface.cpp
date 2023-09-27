#include "myvk/Surface.hpp"

namespace myvk {
Ptr<Surface> Surface::Create(const Ptr<Instance> &instance, GLFWwindow *window) {
	auto ret = std::make_shared<Surface>();
	ret->m_instance_ptr = instance;
	ret->m_window = window;
	if (glfwCreateWindowSurface(ret->m_instance_ptr->GetHandle(), window, nullptr, &ret->m_surface) != VK_SUCCESS)
		return nullptr;
	return ret;
}

Surface::~Surface() {
	if (m_surface)
		vkDestroySurfaceKHR(m_instance_ptr->GetHandle(), m_surface, nullptr);
}
} // namespace myvk
