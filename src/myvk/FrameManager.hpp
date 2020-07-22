#ifndef MYVK_FRAME_MANAGER_HPP
#define MYVK_FRAME_MANAGER_HPP

#include "Swapchain.hpp"
#include "Fence.hpp"
#include "Semaphore.hpp"

namespace myvk {
	class FrameManager {
	private:
		uint32_t m_current_frame{0}, m_frame_count;
		std::vector<Fence *> m_image_fences;
		std::vector<std::shared_ptr<Fence>> m_frame_fences;
		std::vector<std::shared_ptr<Semaphore>> m_render_done_semaphores, m_acquire_done_semaphores;
	public:
		void Initialize(const std::shared_ptr<Swapchain> &swapchain, uint32_t frame_count = 3);

		void BeforeAcquire();

		void AfterAcquire(uint32_t image_index);

		void BeforeSubmit() const;

		const std::shared_ptr<Semaphore>
		&GetAcquireDoneSemaphorePtr() const { return m_acquire_done_semaphores[m_current_frame]; };

		const std::shared_ptr<Semaphore> &
		GetRenderDoneSemaphorePtr() const { return m_render_done_semaphores[m_current_frame]; };

		const std::shared_ptr<Fence> &GetFrameFencePtr() const { return m_frame_fences[m_current_frame]; }

		uint32_t GetCurrentFrame() const { return m_current_frame; }
	};
}


#endif
