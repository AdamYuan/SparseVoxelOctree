#include "Queue.hpp"
#include <memory>

namespace myvk {
	std::shared_ptr<Queue> Queue::create(const std::shared_ptr<Device> &device,
										 uint32_t family_index,
										 uint32_t queue_index) {
		std::shared_ptr<Queue> ret = std::make_shared<Queue>();
		ret->m_device_ptr = device;
		ret->m_family_index = family_index;
		vkGetDeviceQueue(device->GetHandle(), ret->m_family_index, queue_index, &ret->m_queue);
		return ret;
	}

	VkResult Queue::WaitIdle() const {
		return vkQueueWaitIdle(m_queue);
	}

	std::shared_ptr<PresentQueue>
	PresentQueue::create(const std::shared_ptr<Device> &device,
						 const std::shared_ptr<Surface> &surface, uint32_t family_index,
						 uint32_t queue_index) {
		std::shared_ptr<PresentQueue> ret = std::make_shared<PresentQueue>();
		ret->m_device_ptr = device;
		ret->m_surface_ptr = surface;
		ret->m_family_index = family_index;
		vkGetDeviceQueue(device->GetHandle(), ret->m_family_index, queue_index, &ret->m_queue);
		return ret;
	}
} // namespace myvk
