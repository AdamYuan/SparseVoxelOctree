#ifndef MYVK_FENCE_HPP
#define MYVK_FENCE_HPP

#include "DeviceObjectBase.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {
class Fence : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;
	VkFence m_fence{VK_NULL_HANDLE};

public:
	static std::shared_ptr<Fence> Create(const std::shared_ptr<Device> &device, VkFenceCreateFlags flags = 0);

	bool Signaled() const;

	VkFence GetHandle() const { return m_fence; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	VkResult Wait(uint64_t timeout = UINT64_MAX) const;

	VkResult Reset() const;

	~Fence();
};

class FenceGroup {
private:
	VkDevice m_device{};
	std::vector<VkFence> m_fences;

public:
	explicit FenceGroup(const std::vector<std::shared_ptr<Fence>> &fences);

	FenceGroup(const std::initializer_list<std::shared_ptr<Fence>> &fences);

	void Initialize(const std::vector<std::shared_ptr<Fence>> &fences);

	uint32_t GetCount() const { return m_fences.size(); }

	const VkFence *GetFencesPtr() const { return m_fences.data(); }

	VkResult Wait(VkBool32 wait_all = VK_TRUE, uint64_t timeout = UINT64_MAX) const;

	VkResult Reset() const;
};
} // namespace myvk

#endif
