#ifndef MYVK_PHYSICAL_DEVICE_HPP
#define MYVK_PHYSICAL_DEVICE_HPP

#include "Instance.hpp"

#include <volk.h>
#include <vector>
#include <memory>

namespace myvk {
	class PhysicalDevice {
	private:
		std::shared_ptr<Instance> m_instance_ptr;

		VkPhysicalDevice m_physical_device{nullptr};
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceMemoryProperties m_memory_properties;
		VkPhysicalDeviceFeatures m_features;

		void initialize(const std::shared_ptr<Instance> &instance, VkPhysicalDevice physical_device);

	public:
		static std::vector<std::shared_ptr<PhysicalDevice>> Fetch(const std::shared_ptr<Instance> &instance);

		const std::shared_ptr<Instance> &GetInstancePtr() const { return m_instance_ptr; }

		VkPhysicalDevice GetHandle() const { return m_physical_device; }

		const VkPhysicalDeviceProperties &GetProperties() const { return m_properties; }

		const VkPhysicalDeviceMemoryProperties &GetMemoryProperties() const { return m_memory_properties; }

		const VkPhysicalDeviceFeatures &GetFeatures() const { return m_features; }
	};
}

#endif
