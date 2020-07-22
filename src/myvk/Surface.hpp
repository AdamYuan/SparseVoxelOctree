#ifndef MYVK_SURFACE_HPP
#define MYVK_SURFACE_HPP

#include "Instance.hpp"
#include <memory>
#include <volk.h>
#include <GLFW/glfw3.h>

namespace myvk {
	class Surface {
	private:
		std::shared_ptr<Instance> m_instance_ptr;
		GLFWwindow *m_window{nullptr};

		VkSurfaceKHR m_surface{nullptr};
	public:
		static std::shared_ptr<Surface> Create(const std::shared_ptr<Instance> &instance, GLFWwindow *window);

		VkSurfaceKHR GetHandle() const { return m_surface; }

		GLFWwindow *GetGlfwWindow() const { return m_window; }

		~Surface();
	};
}

#endif
