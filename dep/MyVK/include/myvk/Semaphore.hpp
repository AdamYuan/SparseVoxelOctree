#ifndef MYVK_SEMAPHORE_HPP
#define MYVK_SEMAPHORE_HPP

#include "DeviceObjectBase.hpp"
#include "volk.h"
#include <memory>
#include <vector>

namespace myvk {
class Semaphore : public DeviceObjectBase {
private:
	Ptr<Device> m_device_ptr;

	VkSemaphore m_semaphore{VK_NULL_HANDLE};

public:
	static Ptr<Semaphore> Create(const Ptr<Device> &device);

	VkSemaphore GetHandle() const { return m_semaphore; }

	const Ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~Semaphore() override;
};

class SemaphoreGroup {
private:
	std::vector<VkSemaphore> m_semaphores;

public:
	SemaphoreGroup() = default;

	SemaphoreGroup(const std::initializer_list<Ptr<Semaphore>> &semaphores);

	explicit SemaphoreGroup(const std::vector<Ptr<Semaphore>> &semaphores);

	void Initialize(const std::vector<Ptr<Semaphore>> &semaphores);

	uint32_t GetCount() const { return m_semaphores.size(); }

	const VkSemaphore *GetSemaphoresPtr() const { return m_semaphores.data(); }
};

class SemaphoreStageGroup {
private:
	std::vector<VkSemaphore> m_semaphores;
	std::vector<VkPipelineStageFlags> m_stages;

public:
	SemaphoreStageGroup() = default;

	SemaphoreStageGroup(const std::initializer_list<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	explicit SemaphoreStageGroup(const std::vector<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	void Initialize(const std::vector<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	uint32_t GetCount() const { return m_semaphores.size(); }

	const VkSemaphore *GetSemaphoresPtr() const { return m_semaphores.data(); }

	const VkPipelineStageFlags *GetWaitStagesPtr() const { return m_stages.data(); }
};
} // namespace myvk

#endif
