#ifndef MYVK_PHYSICAL_DEVICE_HPP
#define MYVK_PHYSICAL_DEVICE_HPP

#include "Instance.hpp"

#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {
class Surface;
class PhysicalDevice {
private:
	std::shared_ptr<Instance> m_instance_ptr;

	VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};

	VkPhysicalDeviceProperties m_properties;
	VkPhysicalDeviceMemoryProperties m_memory_properties;
	VkPhysicalDeviceFeatures m_features;
	std::vector<VkQueueFamilyProperties> m_queue_family_properties;

	void initialize(const std::shared_ptr<Instance> &instance, VkPhysicalDevice physical_device);

public:
	static std::vector<std::shared_ptr<PhysicalDevice>> Fetch(const std::shared_ptr<Instance> &instance);

	const std::shared_ptr<Instance> &GetInstancePtr() const { return m_instance_ptr; }

	VkPhysicalDevice GetHandle() const { return m_physical_device; }

	const VkPhysicalDeviceProperties &GetProperties() const { return m_properties; }

	const VkPhysicalDeviceMemoryProperties &GetMemoryProperties() const { return m_memory_properties; }

	const VkPhysicalDeviceFeatures &GetFeatures() const { return m_features; }

	const std::vector<VkQueueFamilyProperties> &GetQueueFamilyProperties() const { return m_queue_family_properties; }

	bool GetSurfaceSupport(uint32_t queue_family_index, const std::shared_ptr<Surface> &surface);
};
} // namespace myvk

#endif
