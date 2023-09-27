#include "myvk/PhysicalDevice.hpp"

namespace myvk {
PhysicalDevice::PhysicalDevice(const Ptr<Instance> &instance, VkPhysicalDevice physical_device) {
	m_instance_ptr = instance;
	m_physical_device = physical_device;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &m_memory_properties);
	{
		VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &m_features.vk11};
		vkGetPhysicalDeviceFeatures2(physical_device, &features2);
		m_features.vk10 = features2.features;
	}
	{
		VkPhysicalDeviceProperties2 properties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &m_properties.vk11};
		vkGetPhysicalDeviceProperties2(physical_device, &properties2);
		m_properties.vk10 = properties2.properties;
	}
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
		m_queue_family_properties.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, m_queue_family_properties.data());
	}
	{
		uint32_t count;
		std::vector<VkExtensionProperties> extensions;
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
		extensions.resize(count);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());
		for (const auto &e : extensions)
			m_extensions[e.extensionName] = e.specVersion;
	}
}

std::vector<Ptr<PhysicalDevice>> PhysicalDevice::Fetch(const Ptr<Instance> &instance) {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance->GetHandle(), &device_count, nullptr);
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance->GetHandle(), &device_count, devices.data());

	std::vector<Ptr<PhysicalDevice>> ret;
	ret.resize(device_count);
	for (uint32_t i = 0; i < device_count; ++i) {
		ret[i] = std::make_shared<PhysicalDevice>(instance, devices[i]);
	}
	return ret;
}
bool PhysicalDevice::GetExtensionSupport(const std::string &extension_name, uint32_t *p_version) const {
	auto it = m_extensions.find(extension_name);
	if (p_version && it != m_extensions.end())
		*p_version = it->second;
	return it != m_extensions.end();
}
PhysicalDeviceFeatures PhysicalDevice::GetDefaultFeatures() const {
	PhysicalDeviceFeatures ret{};
	ret.vk10 = m_features.vk10;
	ret.vk10.robustBufferAccess = VK_FALSE;
	ret.vk12.imagelessFramebuffer = VK_TRUE;
	ret.vk13.synchronization2 = VK_TRUE;
	return ret;
}
bool PhysicalDevice::GetQueueSupport(const QueueSelectorFunc &queue_selector_func) const {
	return !queue_selector_func(GetSelfPtr<PhysicalDevice>()).empty();
}

} // namespace myvk
