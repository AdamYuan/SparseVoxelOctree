#include "myvk/Semaphore.hpp"

namespace myvk {
Ptr<Semaphore> Semaphore::Create(const Ptr<Device> &device) {
	auto ret = std::make_shared<Semaphore>();
	ret->m_device_ptr = device;

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(device->GetHandle(), &info, nullptr, &ret->m_semaphore) != VK_SUCCESS)
		return nullptr;
	return ret;
}

Semaphore::~Semaphore() {
	if (m_semaphore)
		vkDestroySemaphore(m_device_ptr->GetHandle(), m_semaphore, nullptr);
}

SemaphoreGroup::SemaphoreGroup(const std::initializer_list<Ptr<Semaphore>> &semaphores) { Initialize(semaphores); }

SemaphoreGroup::SemaphoreGroup(const std::vector<Ptr<Semaphore>> &semaphores) { Initialize(semaphores); }

void SemaphoreGroup::Initialize(const std::vector<Ptr<Semaphore>> &semaphores) {
	m_semaphores.clear();
	for (const auto &i : semaphores)
		m_semaphores.push_back(i->GetHandle());
}

SemaphoreStageGroup::SemaphoreStageGroup(
    const std::initializer_list<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores) {
	Initialize(stage_semaphores);
}

SemaphoreStageGroup::SemaphoreStageGroup(
    const std::vector<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores) {
	Initialize(stage_semaphores);
}

void SemaphoreStageGroup::Initialize(
    const std::vector<std::pair<Ptr<Semaphore>, VkPipelineStageFlags>> &stage_semaphores) {
	m_semaphores.clear();
	m_stages.clear();
	for (const auto &i : stage_semaphores) {
		m_semaphores.push_back(i.first->GetHandle());
		m_stages.push_back(i.second);
	}
}
} // namespace myvk
