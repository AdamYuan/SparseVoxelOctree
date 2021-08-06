#include "Queue.hpp"
#include <memory>

namespace myvk {
std::shared_ptr<UniqueQueue> UniqueQueue::Create(const std::shared_ptr<Device> &device, uint32_t family_index,
                                                 uint32_t queue_index) {
	std::shared_ptr<UniqueQueue> ret = std::make_shared<UniqueQueue>();
	ret->m_device_ptr = device;
	ret->m_family_index = family_index;
	vkGetDeviceQueue(device->GetHandle(), family_index, queue_index, &ret->m_queue);
	return ret;
}

std::shared_ptr<Queue> Queue::Create(const std::shared_ptr<UniqueQueue> &unique_queue) {
	std::shared_ptr<Queue> ret = std::make_shared<Queue>();
	ret->m_unique_queue_ptr = unique_queue;
	return ret;
}

VkResult Queue::WaitIdle() const {
	std::lock_guard<std::mutex> lock_guard{GetMutex()};
	return vkQueueWaitIdle(m_unique_queue_ptr->m_queue);
}

std::shared_ptr<PresentQueue> PresentQueue::Create(const std::shared_ptr<UniqueQueue> &unique_queue,
                                                   const std::shared_ptr<Surface> &surface) {
	std::shared_ptr<PresentQueue> ret = std::make_shared<PresentQueue>();
	ret->m_unique_queue_ptr = unique_queue;
	ret->m_surface_ptr = surface;
	return ret;
}
} // namespace myvk
