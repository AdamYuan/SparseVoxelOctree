#ifndef MYVK_QUEUE_HPP
#define MYVK_QUEUE_HPP

#include "DeviceObjectBase.hpp"

#include <cinttypes>
#include <memory>
#include <mutex>
#include <volk.h>

namespace myvk {

class UniqueQueue : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkQueue m_queue{VK_NULL_HANDLE};
	uint32_t m_family_index;
	std::mutex m_mutex;

public:
	static std::shared_ptr<UniqueQueue> Create(const std::shared_ptr<Device> &device, uint32_t family_index,
	                                           uint32_t queue_index);
	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	friend class Queue;
	friend class PresentQueue;
};

class Queue : public DeviceObjectBase { // only can be created with device creation
protected:
	std::shared_ptr<UniqueQueue> m_unique_queue_ptr;

public:
	static std::shared_ptr<Queue> Create(const std::shared_ptr<UniqueQueue> &unique_queue);

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_unique_queue_ptr->GetDevicePtr(); }

	VkQueue GetHandle() const { return m_unique_queue_ptr->m_queue; }

	uint32_t GetFamilyIndex() const { return m_unique_queue_ptr->m_family_index; }

	std::mutex &GetMutex() const { return m_unique_queue_ptr->m_mutex; }

	VkResult WaitIdle() const;
};

class Surface;

class PresentQueue : public Queue { // only can be created with device creation
private:
	std::shared_ptr<Surface> m_surface_ptr;

public:
	static std::shared_ptr<PresentQueue> Create(const std::shared_ptr<UniqueQueue> &unique_queue,
	                                            const std::shared_ptr<Surface> &surface);

	const std::shared_ptr<Surface> &GetSurfacePtr() const { return m_surface_ptr; }
};
} // namespace myvk

#endif
