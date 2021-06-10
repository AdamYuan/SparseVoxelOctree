#ifndef MYVK_INSTANCE_HPP
#define MYVK_INSTANCE_HPP

#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {
constexpr const char *kValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};

class Instance {
private:
	VkInstance m_instance{VK_NULL_HANDLE};
	VkDebugUtilsMessengerEXT m_debug_messenger{VK_NULL_HANDLE};

public:
	static std::shared_ptr<Instance>
	CreateWithGlfwExtensions(bool use_validation_layer = false,
	                         PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = nullptr);

	VkInstance GetHandle() const { return m_instance; }

	~Instance();
};
} // namespace myvk

#endif
