#ifndef MYVK_DEVICE_HPP
#define MYVK_DEVICE_HPP

#include "DeviceCreateInfo.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace myvk {
class Device {
private:
	std::shared_ptr<PhysicalDevice> m_physical_device_ptr;

	VkDevice m_device{VK_NULL_HANDLE};
	VkPipelineCache m_pipeline_cache{VK_NULL_HANDLE};
	VmaAllocator m_allocator{VK_NULL_HANDLE};

	VkResult create_allocator();

	VkResult create_device(const std::vector<VkDeviceQueueCreateInfo> &queue_create_infos,
	                       const std::vector<const char *> &extensions, void *p_next);

	VkResult create_pipeline_cache();

public:
	static std::shared_ptr<Device> Create(const DeviceCreateInfo &device_create_info, void *p_next = nullptr);

	VmaAllocator GetAllocatorHandle() const { return m_allocator; }

	VkPipelineCache GetPipelineCacheHandle() const { return m_pipeline_cache; }

	const std::shared_ptr<PhysicalDevice> &GetPhysicalDevicePtr() const { return m_physical_device_ptr; }

	VkDevice GetHandle() const { return m_device; }

	VkResult WaitIdle() const;

	~Device();
};
} // namespace myvk

#endif
