#ifndef MYVK_SWAPCHAIN_HPP
#define MYVK_SWAPCHAIN_HPP

#include "DeviceObjectBase.hpp"
#include "Fence.hpp"
#include "Queue.hpp"
#include "Semaphore.hpp"
#include "Surface.hpp"

#include <cinttypes>
#include <memory>
#include <vector>
#include <volk.h>

namespace myvk {
class Swapchain : public DeviceObjectBase {
private:
	std::shared_ptr<Queue> m_graphics_queue_ptr;
	std::shared_ptr<PresentQueue> m_present_queue_ptr;

	VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};

	VkSwapchainCreateInfoKHR m_swapchain_create_info;
	uint32_t m_image_count;

public:
	static std::shared_ptr<Swapchain> Create(const std::shared_ptr<Queue> &graphics_queue,
	                                         const std::shared_ptr<PresentQueue> &present_queue, bool use_vsync);

	static std::shared_ptr<Swapchain> Create(const std::shared_ptr<Swapchain> &old_swapchain);

	VkResult AcquireNextImage(uint32_t *p_image_index, const std::shared_ptr<Semaphore> &signal_semaphore,
	                          const std::shared_ptr<Fence> &signal_fence) const;

	VkResult Present(uint32_t image_index, const SemaphoreGroup &wait_semaphores) const;

	uint32_t GetImageCount() const { return m_image_count; }

	VkSwapchainKHR GetHandle() const { return m_swapchain; };

	const std::shared_ptr<Queue> &GetGraphicsQueuePtr() const { return m_graphics_queue_ptr; }

	const std::shared_ptr<PresentQueue> &GetPresentQueuePtr() const { return m_present_queue_ptr; }

	const std::shared_ptr<Device> &GetDevicePtr() const override { return m_graphics_queue_ptr->GetDevicePtr(); };

	VkFormat GetImageFormat() const { return m_swapchain_create_info.imageFormat; }

	VkColorSpaceKHR GetImageColorSpace() const { return m_swapchain_create_info.imageColorSpace; }

	VkExtent2D GetExtent() const { return m_swapchain_create_info.imageExtent; }

	~Swapchain();
};
} // namespace myvk

#endif
