#include "FrameManager.hpp"

namespace myvk {
void FrameManager::Initialize(const std::shared_ptr<Swapchain> &swapchain, uint32_t frame_count) {
	m_frame_count = frame_count;
	m_image_fences.resize(swapchain->GetImageCount(), nullptr);

	m_frame_fences.resize(frame_count);
	m_render_done_semaphores.resize(frame_count);
	m_acquire_done_semaphores.resize(frame_count);

	for (uint32_t i = 0; i < frame_count; ++i) {
		m_frame_fences[i] = Fence::Create(swapchain->GetDevicePtr(), VK_FENCE_CREATE_SIGNALED_BIT);
		m_render_done_semaphores[i] = Semaphore::Create(swapchain->GetDevicePtr());
		m_acquire_done_semaphores[i] = Semaphore::Create(swapchain->GetDevicePtr());
	}
}

void FrameManager::BeforeAcquire() {
	m_current_frame = (m_current_frame + 1u) % m_frame_count;
	m_frame_fences[m_current_frame]->Wait();
}

void FrameManager::AfterAcquire(uint32_t image_index) {
	if (m_image_fences[image_index])
		m_image_fences[image_index]->Wait();
	m_image_fences[image_index] = m_frame_fences[m_current_frame].get();
}

void FrameManager::BeforeSubmit() const { m_frame_fences[m_current_frame]->Reset(); }
} // namespace myvk
