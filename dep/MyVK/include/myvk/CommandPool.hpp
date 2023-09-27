#ifndef MYVK_COMMAND_POOL_HPP
#define MYVK_COMMAND_POOL_HPP

#include "DeviceObjectBase.hpp"
#include "Queue.hpp"
#include "volk.h"
#include <cinttypes>
#include <memory>

namespace myvk {
class CommandPool : public DeviceObjectBase {
private:
	Ptr<Queue> m_queue_ptr;

	VkCommandPool m_command_pool{VK_NULL_HANDLE};

public:
	static Ptr<CommandPool> Create(const Ptr<Queue> &queue, VkCommandPoolCreateFlags flags = 0);

	VkResult Reset(VkCommandPoolResetFlags flags = 0) const;

	VkCommandPool GetHandle() const { return m_command_pool; }

	const Ptr<Queue> &GetQueuePtr() const { return m_queue_ptr; }

	const Ptr<Device> &GetDevicePtr() const override { return m_queue_ptr->GetDevicePtr(); };

	~CommandPool() override;
};
} // namespace myvk

#endif
