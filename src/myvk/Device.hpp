#ifndef MYVK_DEVICE_HPP
#define MYVK_DEVICE_HPP

#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <memory>
#include "Surface.hpp"
#include "DeviceCreateInfo.hpp"
#include "PhysicalDevice.hpp"

namespace myvk {
	class Device {
		private:
			std::shared_ptr<PhysicalDevice> m_physical_device_ptr;

			VkDevice m_device{nullptr};
			VmaAllocator m_allocator{nullptr};

			bool create_allocator();
			bool create_device(const std::vector<VkDeviceQueueCreateInfo> &queue_create_infos, const std::vector<const char *> &extensions);
		public:
			static std::shared_ptr<Device> Create(const DeviceCreateInfo &device_create_info);
			VmaAllocator GetAllocatorHandle() const { return m_allocator; }
			const std::shared_ptr<PhysicalDevice> &GetPhysicalDevicePtr() const { return m_physical_device_ptr; }
			VkDevice GetHandle() const { return m_device; }

			VkResult WaitIdle() const;

			~Device();
	};
}

#endif
