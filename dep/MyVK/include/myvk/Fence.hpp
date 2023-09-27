#ifndef MYVK_FENCE_HPP
#define MYVK_FENCE_HPP

#include "DeviceObjectBase.hpp"
#include "volk.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace myvk {
class Fence : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;
	VkFence m_fence{VK_NULL_HANDLE};

public:
	static Ptr<Fence> Create(const Ptr<Device> &device, VkFenceCreateFlags flags = 0);

	bool Signaled() const;

	VkFence GetHandle() const { return m_fence; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	VkResult Wait(uint64_t timeout = UINT64_MAX) const;

	VkResult Reset() const;

	~Fence() override;
};

class FenceGroup {
private:
	VkDevice m_device{};
	std::vector<VkFence> m_fences;

public:
	explicit FenceGroup(const std::vector<Ptr<Fence>> &fences);

	FenceGroup(const std::initializer_list<Ptr<Fence>> &fences);

	void Initialize(const std::vector<Ptr<Fence>> &fences);

	uint32_t GetCount() const { return m_fences.size(); }

	const VkFence *GetFencesPtr() const { return m_fences.data(); }

	VkResult Wait(VkBool32 wait_all = VK_TRUE, uint64_t timeout = UINT64_MAX) const;

	VkResult Reset() const;
};
} // namespace myvk

#endif
