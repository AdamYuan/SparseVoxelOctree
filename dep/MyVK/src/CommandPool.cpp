#include "myvk/CommandPool.hpp"

namespace myvk {
Ptr<CommandPool> CommandPool::Create(const Ptr<Queue> &queue, VkCommandPoolCreateFlags flags) {
	auto ret = std::make_shared<CommandPool>();
	ret->m_queue_ptr = queue;

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue->GetFamilyIndex();
	create_info.flags = flags;
	if (vkCreateCommandPool(queue->GetDevicePtr()->GetHandle(), &create_info, nullptr, &ret->m_command_pool) !=
	    VK_SUCCESS)
		return nullptr;
	return ret;
}

VkResult CommandPool::Reset(VkCommandPoolResetFlags flags) const {
	return vkResetCommandPool(GetDevicePtr()->GetHandle(), m_command_pool, flags);
}

CommandPool::~CommandPool() {
	if (m_command_pool)
		vkDestroyCommandPool(m_queue_ptr->GetDevicePtr()->GetHandle(), m_command_pool, nullptr);
}
} // namespace myvk
