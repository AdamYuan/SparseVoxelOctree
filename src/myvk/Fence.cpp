#include "Fence.hpp"

namespace myvk {
std::shared_ptr<Fence> Fence::Create(const std::shared_ptr<Device> &device, VkFenceCreateFlags flags) {
	std::shared_ptr<Fence> ret = std::make_shared<Fence>();
	ret->m_device_ptr = device;

	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = flags;
	if (vkCreateFence(device->GetHandle(), &info, nullptr, &ret->m_fence) != VK_SUCCESS)
		return nullptr;

	return ret;
}

bool Fence::Signaled() const { return vkGetFenceStatus(m_device_ptr->GetHandle(), m_fence) == VK_SUCCESS; }

Fence::~Fence() {
	if (m_fence)
		vkDestroyFence(m_device_ptr->GetHandle(), m_fence, nullptr);
}

VkResult Fence::Wait(uint64_t timeout) const {
	return vkWaitForFences(m_device_ptr->GetHandle(), 1, &m_fence, VK_TRUE, timeout);
}

VkResult Fence::Reset() const { return vkResetFences(m_device_ptr->GetHandle(), 1, &m_fence); }

FenceGroup::FenceGroup(const std::initializer_list<std::shared_ptr<Fence>> &fences) { Initialize(fences); }

FenceGroup::FenceGroup(const std::vector<std::shared_ptr<Fence>> &fences) { Initialize(fences); }

void FenceGroup::Initialize(const std::vector<std::shared_ptr<Fence>> &fences) {
	m_fences.clear();
	m_device = fences.front()->GetDevicePtr()->GetHandle();
	for (const auto &i : fences)
		m_fences.push_back(i->GetHandle());
}

VkResult FenceGroup::Reset() const { return vkResetFences(m_device, m_fences.size(), m_fences.data()); }

VkResult FenceGroup::Wait(VkBool32 wait_all, uint64_t timeout) const {
	return vkWaitForFences(m_device, m_fences.size(), m_fences.data(), wait_all, timeout);
}
} // namespace myvk
