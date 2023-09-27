#include "myvk/Queue.hpp"
#include <memory>

namespace myvk {
Ptr<UniqueQueue> UniqueQueue::Create(const Ptr<Device> &device, uint32_t family_index, uint32_t queue_index) {
	auto ret = std::make_shared<UniqueQueue>();
	ret->m_device_ptr = device;
	ret->m_family_index = family_index;
	vkGetDeviceQueue(device->GetHandle(), family_index, queue_index, &ret->m_queue);
	return ret;
}

Ptr<Queue> Queue::Create(const Ptr<UniqueQueue> &unique_queue) {
	auto ret = std::make_shared<Queue>();
	ret->m_unique_queue_ptr = unique_queue;
	return ret;
}

VkResult Queue::WaitIdle() const {
	std::lock_guard<std::mutex> lock_guard{GetMutex()};
	return vkQueueWaitIdle(m_unique_queue_ptr->m_queue);
}

Ptr<PresentQueue> PresentQueue::Create(const Ptr<UniqueQueue> &unique_queue, const Ptr<Surface> &surface) {
	auto ret = std::make_shared<PresentQueue>();
	ret->m_unique_queue_ptr = unique_queue;
	ret->m_surface_ptr = surface;
	return ret;
}
} // namespace myvk
