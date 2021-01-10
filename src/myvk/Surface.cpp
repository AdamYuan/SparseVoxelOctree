#include "Surface.hpp"

namespace myvk {
std::shared_ptr<Surface> Surface::Create(const std::shared_ptr<Instance> &instance, GLFWwindow *window) {
	std::shared_ptr<Surface> ret = std::make_shared<Surface>();
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
