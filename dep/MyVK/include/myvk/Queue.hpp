#ifndef MYVK_QUEUE_HPP
#define MYVK_QUEUE_HPP

#include "DeviceObjectBase.hpp"

#include "volk.h"
#include <cinttypes>
#include <memory>
#include <mutex>

namespace myvk {

class UniqueQueue : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	VkQueue m_queue{VK_NULL_HANDLE};
	uint32_t m_family_index;
	mutable std::mutex m_mutex;

public:
	static Ptr<UniqueQueue> Create(const Ptr<Device> &device, uint32_t family_index, uint32_t queue_index);
	~UniqueQueue() override = default;
	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	friend class Queue;
	friend class PresentQueue;
};

class Queue : public DeviceObjectBase { // only can be created with device creation
protected:
	Ptr<UniqueQueue> m_unique_queue_ptr;

public:
	static Ptr<Queue> Create(const Ptr<UniqueQueue> &unique_queue);

	~Queue() override = default;

	const Ptr<Device> &GetDevicePtr() const override { return m_unique_queue_ptr->GetDevicePtr(); }

	VkQueue GetHandle() const { return m_unique_queue_ptr->m_queue; }

	uint32_t GetFamilyIndex() const { return m_unique_queue_ptr->m_family_index; }

	std::mutex &GetMutex() const { return m_unique_queue_ptr->m_mutex; }

	VkResult WaitIdle() const;
};

class Surface;

class PresentQueue : public Queue { // only can be created with device creation
private:
	Ptr<Surface> m_surface_ptr;

public:
	static Ptr<PresentQueue> Create(const Ptr<UniqueQueue> &unique_queue, const Ptr<Surface> &surface);

	~PresentQueue() override = default;

	const Ptr<Surface> &GetSurfacePtr() const { return m_surface_ptr; }
};
} // namespace myvk

#endif
