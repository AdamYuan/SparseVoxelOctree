#ifndef MYVK_SEMAPHORE_HPP
#define MYVK_SEMAPHORE_HPP

#include "DeviceObjectBase.hpp"
#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {
class Semaphore : public DeviceObjectBase {
private:
	std::shared_ptr<Device> m_device_ptr;

	VkSemaphore m_semaphore{VK_NULL_HANDLE};

public:
	static std::shared_ptr<Semaphore> Create(const std::shared_ptr<Device> &device);

	VkSemaphore GetHandle() const { return m_semaphore; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_device_ptr; }

	~Semaphore();
};

class SemaphoreGroup {
private:
	std::vector<VkSemaphore> m_semaphores;

public:
	SemaphoreGroup() = default;

	SemaphoreGroup(const std::initializer_list<std::shared_ptr<Semaphore>> &semaphores);

	explicit SemaphoreGroup(const std::vector<std::shared_ptr<Semaphore>> &semaphores);

	void Initialize(const std::vector<std::shared_ptr<Semaphore>> &semaphores);

	uint32_t GetCount() const { return m_semaphores.size(); }

	const VkSemaphore *GetSemaphoresPtr() const { return m_semaphores.data(); }
};

class SemaphoreStageGroup {
private:
	std::vector<VkSemaphore> m_semaphores;
	std::vector<VkPipelineStageFlags> m_stages;

public:
	SemaphoreStageGroup() = default;

	SemaphoreStageGroup(
	    const std::initializer_list<std::pair<std::shared_ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	explicit SemaphoreStageGroup(
	    const std::vector<std::pair<std::shared_ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	void Initialize(const std::vector<std::pair<std::shared_ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores);

	uint32_t GetCount() const { return m_semaphores.size(); }

	const VkSemaphore *GetSemaphoresPtr() const { return m_semaphores.data(); }

	const VkPipelineStageFlags *GetWaitStagesPtr() const { return m_stages.data(); }
};
} // namespace myvk

#endif
