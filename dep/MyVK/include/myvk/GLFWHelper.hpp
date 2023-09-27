#ifdef MYVK_ENABLE_GLFW

#ifndef MYVK_GLFW_HELPER_HPP
#define MYVK_GLFW_HELPER_HPP

#include <GLFW/glfw3.h>

namespace myvk {
inline GLFWwindow *GLFWCreateWindow(const char *title, uint32_t width, uint32_t height, bool resizable = true) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, resizable);
	return glfwCreateWindow(width, height, title, nullptr, nullptr);
}
} // namespace myvk

#endif

#endif
