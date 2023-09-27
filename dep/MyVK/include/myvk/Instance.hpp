#ifndef MYVK_INSTANCE_HPP
#define MYVK_INSTANCE_HPP

#include "Base.hpp"
#include "Ptr.hpp"
#include "volk.h"
#include <memory>
#include <vector>

namespace myvk {
constexpr const char *kValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};

class Instance : public Base {
private:
	VkInstance m_instance{VK_NULL_HANDLE};
	VkDebugUtilsMessengerEXT m_debug_messenger{VK_NULL_HANDLE};

public:
	static Ptr<Instance> Create(const std::vector<const char *> &extensions, bool use_validation_layer = false,
	                            PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = nullptr);
#ifdef MYVK_ENABLE_GLFW
	static Ptr<Instance> CreateWithGlfwExtensions(bool use_validation_layer = false,
	                                              PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = nullptr);
#endif

	VkInstance GetHandle() const { return m_instance; }

	~Instance() override;
};
} // namespace myvk

#endif
