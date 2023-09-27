#ifndef MYVK_PHYSICAL_DEVICE_HPP
#define MYVK_PHYSICAL_DEVICE_HPP

#include "Base.hpp"
#include "Instance.hpp"
#include "QueueSelector.hpp"

#include "volk.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace myvk {
class Surface;
struct PhysicalDeviceProperties {
	VkPhysicalDeviceProperties vk10{};
	VkPhysicalDeviceVulkan11Properties vk11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES, &vk12};
	VkPhysicalDeviceVulkan12Properties vk12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES, &vk13};
	VkPhysicalDeviceVulkan13Properties vk13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
	inline PhysicalDeviceProperties() = default;
	inline PhysicalDeviceProperties(const PhysicalDeviceProperties &r)
	    : vk10{r.vk10}, vk11{r.vk11}, vk12{r.vk12}, vk13{r.vk13} {
		vk11.pNext = &vk12;
		vk12.pNext = &vk13;
		vk13.pNext = nullptr;
	}
	inline PhysicalDeviceProperties &operator=(const PhysicalDeviceProperties &r) {
		vk10 = r.vk10;
		vk11 = r.vk11;
		vk12 = r.vk12;
		vk13 = r.vk13;
		vk11.pNext = &vk12;
		vk12.pNext = &vk13;
		vk13.pNext = nullptr;
		return *this;
	}
};
struct PhysicalDeviceFeatures {
	VkPhysicalDeviceFeatures vk10{};
	VkPhysicalDeviceVulkan11Features vk11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &vk12};
	VkPhysicalDeviceVulkan12Features vk12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &vk13};
	VkPhysicalDeviceVulkan13Features vk13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	inline PhysicalDeviceFeatures() = default;
	inline PhysicalDeviceFeatures(const PhysicalDeviceFeatures &r)
	    : vk10{r.vk10}, vk11{r.vk11}, vk12{r.vk12}, vk13{r.vk13} {
		vk11.pNext = &vk12;
		vk12.pNext = &vk13;
		vk13.pNext = nullptr;
	}
	inline PhysicalDeviceFeatures &operator=(const PhysicalDeviceFeatures &r) {
		vk10 = r.vk10;
		vk11 = r.vk11;
		vk12 = r.vk12;
		vk13 = r.vk13;
		vk11.pNext = &vk12;
		vk12.pNext = &vk13;
		vk13.pNext = nullptr;
		return *this;
	}
};
class PhysicalDevice : public Base {
private:
	Ptr<Instance> m_instance_ptr;

	VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};

	VkPhysicalDeviceMemoryProperties m_memory_properties{};
	PhysicalDeviceFeatures m_features;
	PhysicalDeviceProperties m_properties;
	std::vector<VkQueueFamilyProperties> m_queue_family_properties;
	std::map<std::string, uint32_t> m_extensions;

public:
	PhysicalDevice(const Ptr<Instance> &instance, VkPhysicalDevice physical_device);
	inline ~PhysicalDevice() override = default;
	static std::vector<Ptr<PhysicalDevice>> Fetch(const Ptr<Instance> &instance);

	inline const Ptr<Instance> &GetInstancePtr() const { return m_instance_ptr; }
	inline VkPhysicalDevice GetHandle() const { return m_physical_device; }
	inline const PhysicalDeviceProperties &GetProperties() const { return m_properties; }
	inline const PhysicalDeviceFeatures &GetFeatures() const { return m_features; }
	inline const VkPhysicalDeviceMemoryProperties &GetMemoryProperties() const { return m_memory_properties; }
	inline const std::vector<VkQueueFamilyProperties> &GetQueueFamilyProperties() const {
		return m_queue_family_properties;
	}
	bool GetExtensionSupport(const std::string &extension_name, uint32_t *p_version = nullptr) const;
	bool GetQueueSupport(const QueueSelectorFunc &queue_selector_func) const;
	PhysicalDeviceFeatures GetDefaultFeatures() const;
#ifdef MYVK_ENABLE_GLFW
	bool GetQueueSurfaceSupport(uint32_t queue_family_index, const Ptr<Surface> &surface) const;
#endif
};
} // namespace myvk

#endif
