#include "Instance.hpp"

#include <GLFW/glfw3.h>

namespace myvk {
std::shared_ptr<Instance> Instance::CreateWithGlfwExtensions(bool use_validation_layer,
                                                             PFN_vkDebugUtilsMessengerCallbackEXT debug_callback) {
	std::shared_ptr<Instance> ret = std::make_shared<Instance>();

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	uint32_t glfw_extension_count = 0;
	const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
	if (use_validation_layer)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	create_info.enabledExtensionCount = extensions.size();
	create_info.ppEnabledExtensionNames = extensions.data();

	VkValidationFeatureEnableEXT validation_features_enabled[] = {
	    VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
	VkValidationFeaturesEXT validation_features = {};
	validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validation_features.enabledValidationFeatureCount =
	    sizeof(validation_features_enabled) / sizeof(VkValidationFeatureEnableEXT);
	validation_features.pEnabledValidationFeatures = validation_features_enabled;

	if (use_validation_layer) {
		create_info.enabledLayerCount = sizeof(kValidationLayers) / sizeof(const char *);
		create_info.ppEnabledLayerNames = kValidationLayers;

		validation_features.pNext = create_info.pNext;
		create_info.pNext = &validation_features;
	} else
		create_info.enabledLayerCount = 0;

	if (vkCreateInstance(&create_info, nullptr, &ret->m_instance) != VK_SUCCESS)
		return nullptr;
	volkLoadInstanceOnly(ret->m_instance);

	if (use_validation_layer && debug_callback) {
		VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_info.messageSeverity =
		    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_info.pfnUserCallback = debug_callback;
		debug_info.pUserData = nullptr;

		if (vkCreateDebugUtilsMessengerEXT(ret->m_instance, &debug_info, nullptr, &ret->m_debug_messenger) !=
		    VK_SUCCESS)
			return nullptr;
	}

	return ret;
}

Instance::~Instance() {
	if (m_instance) {
		if (m_debug_messenger)
			vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}
}
} // namespace myvk
