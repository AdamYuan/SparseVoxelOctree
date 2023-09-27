#ifndef MYVK_DEVICE_HPP
#define MYVK_DEVICE_HPP

#include "PhysicalDevice.hpp"
#include "Ptr.hpp"
#include "QueueSelector.hpp"
#include "Surface.hpp"
#include "vk_mem_alloc.h"
#include "volk.h"
#include <memory>
#include <vector>

namespace myvk {
class Device : public Base {
private:
	Ptr<PhysicalDevice> m_physical_device_ptr;

	PhysicalDeviceFeatures m_features;
	VkDevice m_device{VK_NULL_HANDLE};
	VkPipelineCache m_pipeline_cache{VK_NULL_HANDLE};
	VmaAllocator m_allocator{VK_NULL_HANDLE};

	VkResult create_allocator();

	VkResult create_device(const std::vector<VkDeviceQueueCreateInfo> &queue_create_infos,
	                       const std::vector<const char *> &extensions, const PhysicalDeviceFeatures &features);

	VkResult create_pipeline_cache();

public:
	static Ptr<Device> Create(const Ptr<PhysicalDevice> &physical_device, const QueueSelectorFunc &queue_selector_func,
	                          const PhysicalDeviceFeatures &features, const std::vector<const char *> &extensions);

	inline VmaAllocator GetAllocatorHandle() const { return m_allocator; }
	inline VkPipelineCache GetPipelineCacheHandle() const { return m_pipeline_cache; }
	inline const Ptr<PhysicalDevice> &GetPhysicalDevicePtr() const { return m_physical_device_ptr; }
	inline VkDevice GetHandle() const { return m_device; }
	inline const PhysicalDeviceFeatures &GetEnabledFeatures() const { return m_features; }

	VkResult WaitIdle() const;

	~Device() override;
};
} // namespace myvk

#endif
