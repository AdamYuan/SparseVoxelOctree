#include "PhysicalDevice.hpp"

namespace myvk {
void PhysicalDevice::initialize(const std::shared_ptr<Instance> &instance, VkPhysicalDevice physical_device) {
	m_instance_ptr = instance;
	m_physical_device = physical_device;
	vkGetPhysicalDeviceProperties(physical_device, &m_properties);
	vkGetPhysicalDeviceMemoryProperties(physical_device, &m_memory_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &m_features);
}

std::vector<std::shared_ptr<PhysicalDevice>> PhysicalDevice::Fetch(const std::shared_ptr<Instance> &instance) {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance->GetHandle(), &device_count, nullptr);
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance->GetHandle(), &device_count, devices.data());

	std::vector<std::shared_ptr<PhysicalDevice>> ret;
	ret.resize(device_count);
	for (uint32_t i = 0; i < device_count; ++i) {
		ret[i] = std::make_shared<PhysicalDevice>();
		ret[i]->initialize(instance, devices[i]);
	}
	return ret;
}
} // namespace myvk
